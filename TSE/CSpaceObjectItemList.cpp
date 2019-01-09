//	CSpaceObjectItemList.cpp
//
//	CSpaceObject class
//	Copyright (c) 2018 Kronosaur Productions, LLC. All Rights Reserved.

#include "PreComp.h"

#define FIELD_DESC_ID							CONSTLIT("descID")
#define FIELD_CAN_INSTALL						CONSTLIT("canInstall")
#define FIELD_CAN_REMOVE						CONSTLIT("canRemove")
#define FIELD_PRICE								CONSTLIT("price")
#define FIELD_UPGRADE_INSTALL_ONLY				CONSTLIT("upgradeInstallOnly")

#define PROPERTY_DAMAGED						CONSTLIT("damaged")
#define PROPERTY_ENABLED						CONSTLIT("enabled")
#define PROPERTY_HP								CONSTLIT("hp")
#define PROPERTY_INSTALL_DEVICE_PRICE			CONSTLIT("installDevicePrice")
#define PROPERTY_INSTALL_DEVICE_STATUS			CONSTLIT("installDeviceStatus")
#define PROPERTY_INSTALL_ITEM_STATUS			CONSTLIT("installItemStatus")
#define PROPERTY_REMOVE_DEVICE_PRICE			CONSTLIT("removeDevicePrice")
#define PROPERTY_REMOVE_ITEM_STATUS				CONSTLIT("removeItemStatus")

bool CSpaceObject::AddItem (const CItem &Item, CItem *retResult, CString *retsError)

//	AddItem
//
//	Adds an item to the object. Returns TRUE if we were successful.

	{
	//	Notify any dock screens that we might modify an item
	//	Null item means preserve current selection.

	IDockScreenUI::SModifyItemCtx ModifyCtx;
	OnModifyItemBegin(ModifyCtx, CItem());

	//	Add the item

	CItemListManipulator ObjList(GetItemList());
	ObjList.AddItem(Item);

	//	Update the object

	OnComponentChanged(comCargo);
	OnModifyItemComplete(ModifyCtx, Item);

	//	Return the item.

	if (retResult)
		*retResult = ObjList.GetItemAtCursor();

	return true;
	}

ALERROR CSpaceObject::CreateRandomItems (IItemGenerator *pItems, CSystem *pSystem)

//	CreateRandomItems
//
//	Creates items based on item table
//
//	NOTE: We cannot call GetSystem() because it is not yet set up at the time
//	that we call this (on create). Thus we rely on pSystem being passed in.

	{
	DEBUG_TRY

	if (pItems)
		{
		//	Notify any dock screens that we might modify an item
		//	Null item means preserve current selection.

		IDockScreenUI::SModifyItemCtx ModifyCtx;
		OnModifyItemBegin(ModifyCtx, CItem());

		//	Add the items

		CItemListManipulator ItemList(GetItemList());
		SItemAddCtx Ctx(ItemList);
		Ctx.pSystem = pSystem;
		Ctx.pDest = this;
		Ctx.vPos = GetPos();
		Ctx.iLevel = (Ctx.pSystem ? Ctx.pSystem->GetLevel() : 1);

		pItems->AddItems(Ctx);

		//	Update the object

		OnComponentChanged(comCargo);
		OnModifyItemComplete(ModifyCtx, CItem());
		}

	return NOERROR;

	DEBUG_CATCH
	}

ALERROR CSpaceObject::CreateRandomItems (CXMLElement *pItems, CSystem *pSystem)

//	CreateRandomItems
//
//	Creates items based on lookup

	{
	ALERROR error;

	//	Load the table

	SDesignLoadCtx Ctx;

	IItemGenerator *pGenerator;
	if (error = IItemGenerator::CreateFromXML(Ctx, pItems, &pGenerator))
		return error;

	if (error = pGenerator->OnDesignLoadComplete(Ctx))
		{
		CString sError = strPatternSubst(CONSTLIT("ERROR: Unable to create random items: %s\r\n"), Ctx.sError);
		ReportCrashObj(&sError, this);
		kernelDebugLogString(sError);
		ASSERT(false);
		return error;
		}

	//	Add the items

	CreateRandomItems(pGenerator, pSystem);

	//	Done

	delete pGenerator;
	return NOERROR;
	}

CItem CSpaceObject::GetItemForDevice (CInstalledDevice *pDevice)

//	GetItemForDevice
//
//	Returns the item for the given device

	{
	CItemListManipulator ItemList(GetItemList());
	SetCursorAtDevice(ItemList, pDevice);
	return ItemList.GetItemAtCursor();
	}

ICCItem *CSpaceObject::GetItemProperty (CCodeChainCtx &CCCtx, const CItem &Item, const CString &sName)

//	GetItemProperty
//
//	Returns an item property

	{
	CCodeChain &CC = g_pUniverse->GetCC();

	//	Return the property

	if (strEquals(sName, PROPERTY_INSTALL_DEVICE_PRICE))
		{
		int iPrice;
		if (!GetDeviceInstallPrice(Item, 0, &iPrice))
			return CC.CreateNil();

		return CC.CreateInteger(iPrice);
		}
	else if (strEquals(sName, PROPERTY_INSTALL_DEVICE_STATUS))
		{
		//	We return a structure with the following fields:
		//
		//	canInstall: True or Nil
		//	price: Install price
		//	descID: Message ID for description of install attempt
		//	upgradeInstallOnly: True if we only install on upgrade

		CString sMessageID;
		int iPrice;
		DWORD dwPriceFlags;
		bool bCanInstall = GetDeviceInstallPrice(Item, 0, &iPrice, &sMessageID, &dwPriceFlags);

		//	Create the structure

		ICCItem *pResult = CC.CreateSymbolTable();
		pResult->SetAt(CC, FIELD_CAN_INSTALL, (bCanInstall ? CC.CreateTrue() : CC.CreateNil()));
		pResult->SetIntegerAt(CC, FIELD_PRICE, (bCanInstall ? iPrice : -1));
		if (bCanInstall && (dwPriceFlags & CTradingDesc::PRICE_UPGRADE_INSTALL_ONLY))
			pResult->SetAt(CC, FIELD_UPGRADE_INSTALL_ONLY, CC.CreateTrue());

		if (!sMessageID.IsBlank())
			pResult->SetStringAt(CC, FIELD_DESC_ID, sMessageID);

		return pResult;
		}
	else if (strEquals(sName, PROPERTY_INSTALL_ITEM_STATUS))
		{
		//	We return a structure with the following fields:
		//
		//	canInstall: True or Nil
		//	price: Install price
		//	descID: Message ID for description of install attempt
		//	upgradeInstallOnly: True if we only install on upgrade

		CString sMessageID;
		int iPrice;
		bool bCanInstall;
		DWORD dwPriceFlags = 0;
		if (Item.IsArmor())
			bCanInstall = GetArmorInstallPrice(Item, 0, &iPrice, &sMessageID);
		else if (Item.IsDevice())
			bCanInstall = GetDeviceInstallPrice(Item, 0, &iPrice, &sMessageID, &dwPriceFlags);
		else
			bCanInstall = false;

		//	Create the structure

		ICCItem *pResult = CC.CreateSymbolTable();
		pResult->SetAt(CC, FIELD_CAN_INSTALL, (bCanInstall ? CC.CreateTrue() : CC.CreateNil()));
		pResult->SetIntegerAt(CC, FIELD_PRICE, (bCanInstall ? iPrice : -1));
		if (bCanInstall 
				&& (dwPriceFlags & CTradingDesc::PRICE_UPGRADE_INSTALL_ONLY))
			pResult->SetAt(CC, FIELD_UPGRADE_INSTALL_ONLY, CC.CreateTrue());

		//	NOTE: Message is valid even if we cannot install

		if (!sMessageID.IsBlank())
			pResult->SetStringAt(CC, FIELD_DESC_ID, sMessageID);

		return pResult;
		}
	else if (strEquals(sName, PROPERTY_REMOVE_DEVICE_PRICE))
		{
		int iPrice;
		if (!GetDeviceRemovePrice(Item, 0, &iPrice))
			return CC.CreateNil();

		return CC.CreateInteger(iPrice);
		}
	else if (strEquals(sName, PROPERTY_REMOVE_ITEM_STATUS))
		{
		//	We return a structure with the following fields:
		//
		//	canRemove: True or Nil
		//	price: Remove price
		//	descID: Message ID for description of remove attempt
		//	upgradeInstallOnly: True if we only install/remove on upgrade

		CString sMessageID;
		int iPrice;
		bool bCanRemove;
		DWORD dwPriceFlags = 0;
		if (Item.IsDevice())
			bCanRemove = GetDeviceRemovePrice(Item, 0, &iPrice, &dwPriceFlags);
		else
			bCanRemove = false;

		//	Create the structure

		ICCItem *pResult = CC.CreateSymbolTable();
		pResult->SetAt(CC, FIELD_CAN_REMOVE, (bCanRemove ? CC.CreateTrue() : CC.CreateNil()));
		pResult->SetIntegerAt(CC, FIELD_PRICE, (bCanRemove ? iPrice : -1));
		if (bCanRemove 
				&& (dwPriceFlags & CTradingDesc::PRICE_UPGRADE_INSTALL_ONLY))
			pResult->SetAt(CC, FIELD_UPGRADE_INSTALL_ONLY, CC.CreateTrue());

		if (!sMessageID.IsBlank())
			pResult->SetStringAt(CC, FIELD_DESC_ID, sMessageID);

		return pResult;
		}
	else
		{
		//	Select the item (to make sure that it is part of the object)

		CItemListManipulator ItemList(GetItemList());
		if (!ItemList.SetCursorAtItem(Item))
			return CC.CreateError(CONSTLIT("Item not found on object."));

		CItemCtx Ctx(&Item, this);
		return Item.GetItemProperty(CCCtx, Ctx, sName);
		}
	}

bool CSpaceObject::RemoveItem (const CItem &Item, DWORD dwItemMatchFlags, int iCount, int *retiCountRemoved, CString *retsError)

//	RemoveItem
//
//	Removes the given item.

	{
	if (retiCountRemoved) *retiCountRemoved = 0;

	if (iCount == -1)
		iCount = Item.GetCount();

	//	If not found, then we succeed, but leave count removed as 0.

	CItemListManipulator ObjList(GetItemList());
	if (!ObjList.SetCursorAtItem(Item, dwItemMatchFlags))
		return true;

	//	Can't be installed

	if (ObjList.GetItemAtCursor().IsInstalled())
		{
		if (retsError) *retsError = CONSTLIT("Installed items cannot be removed");
		return false;
		}

	//	Notify any dock screens that we might modify an item

	IDockScreenUI::SModifyItemCtx ModifyCtx;
	OnModifyItemBegin(ModifyCtx, ObjList.GetItemAtCursor());

	//	Remove

	if (retiCountRemoved) *retiCountRemoved = Min(ObjList.GetItemAtCursor().GetCount(), iCount);
	ObjList.DeleteAtCursor(iCount);

	//	Update (empty item indicates that we deleted some items).

	OnComponentChanged(comCargo);
	OnModifyItemComplete(ModifyCtx, CItem());

	return true;
	}

bool CSpaceObject::SetItemData (const CItem &Item, const CString &sField, ICCItem *pValue, int iCount, CItem *retItem, CString *retsError)

//	SetItemData
//
//	Sets the item data. pValue may be NULL, indicating that we should delete the
//	value.

	{
	//	Select the item

	CItemListManipulator ItemList(GetItemList());
	if (!ItemList.SetCursorAtItem(Item))
		{
		*retsError = CONSTLIT("Item not found on object.");
		return false;
		}

	//	Notify any dock screens that we might modify an item

	IDockScreenUI::SModifyItemCtx ModifyCtx;
	OnModifyItemBegin(ModifyCtx, Item);

	//	Set the data

	ItemList.SetDataAtCursor(sField, pValue, iCount);

	//	Update the object

	OnModifyItemComplete(ModifyCtx, ItemList.GetItemAtCursor());

	//	Return the newly changed item

	if (retItem)
		*retItem = ItemList.GetItemAtCursor();

	return true;
	}

bool CSpaceObject::SetItemProperty (const CItem &Item, const CString &sName, ICCItem *pValue, int iCount, CItem *retItem, CString *retsError)

//	SetItemProperty
//
//	Sets the item property.
//
//	NOTE: pValue may be NULL.

	{
	//	Select the item to make sure it exists on this object.

	CItemListManipulator ItemList(GetItemList());
	if (!ItemList.SetCursorAtItem(Item))
		{
		if (retsError) *retsError = CONSTLIT("Item not found on object.");
		return false;
		}

	//	We handle damage differently because we may need to remove enhancements,
	//	etc.

	if (strEquals(sName, PROPERTY_DAMAGED))
		{
		if (pValue && pValue->IsNil())
			RepairItem(ItemList);
		else
			DamageItem(ItemList);
		}

	//	Enabling/disabling needs special handling

	else if (strEquals(sName, PROPERTY_ENABLED))
		{
		CShip *pShip = AsShip();
		if (pShip == NULL)
			{
			if (retsError) *retsError = CONSTLIT("Property not supported.");
			return false;
			}

		if (!ItemList.GetItemAtCursor().IsInstalled())
			{
			if (retsError) *retsError = CONSTLIT("Device is not installed on object.");
			return false;
			}

		//	Parse the parameter

		bool bEnabled;
		bool bSilent;
		if (pValue == NULL)
			{
			bEnabled = true;
			bSilent = false;
			}
		else if (pValue->IsIdentifier())
			{
			if (strEquals(pValue->GetStringValue(), CONSTLIT("silentDisabled")))
				{
				bEnabled = false;
				bSilent = true;
				}
			else if (strEquals(pValue->GetStringValue(), CONSTLIT("silentEnabled")))
				{
				bEnabled = true;
				bSilent = true;
				}
			else if (strEquals(pValue->GetStringValue(), CONSTLIT("disabled")))
				{
				bEnabled = false;
				bSilent = false;
				}
			else
				{
				bEnabled = true;
				bSilent = false;
				}
			}
		else
			{
			bEnabled = !pValue->IsNil();
			bSilent = false;
			}

		//	Do it.

		pShip->EnableDevice(ItemList.GetItemAtCursor().GetInstalled(), bEnabled, bSilent);
		}

	else if (strEquals(sName, PROPERTY_HP))
		{
		int iHP = (pValue ? pValue->GetIntegerValue() : 0);

		if (Item.IsInstalled())
			{
			if (Item.GetType()->GetArmorClass())
				{
				CShip *pShip = AsShip();

				int iArmorSeg = Item.GetInstalled();
				CInstalledArmor *pSection = pShip->GetArmorSection(iArmorSeg);
				if (pSection == NULL)
					{
					if (retsError) *retsError = CONSTLIT("Unable to find armor segment on ship.");
					return false;
					}

				iHP = Min(iHP, pSection->GetMaxHP(this));

				if (iHP < pSection->GetHitPoints())
					{
					DamageDesc Damage(damageGeneric, DiceRange(0, 0, pSection->GetHitPoints() - iHP));
					Damage.SetNoSRSFlash();
					pShip->DamageArmor(iArmorSeg, Damage);
					}
				else if (iHP > pSection->GetHitPoints())
					pShip->RepairArmor(iArmorSeg, iHP - pSection->GetHitPoints());
				}
			else if (Item.GetType()->GetDeviceClass())
				{
				//	Set the data
				//
				//	LATER: At some point we should move the code above that sets
				//	armor HP to be handled by SetPropertyAtCursor. At that point,
				//	we can delete this section and let it go to the default.

				if (!ItemList.SetPropertyAtCursor(this, sName, pValue, iCount, retsError))
					return false;

				//	Update the object

				ItemEnhancementModified(ItemList);
				}
			else
				{
				if (retsError) *retsError = CONSTLIT("Unable to set hit points.");
				return false;
				}
			}
		else
			{
			if (retsError) *retsError = CONSTLIT("Unable to set hit points.");
			return false;
			}
		}

	//	Otherwise, just set the property, but pass enough context (this object)
	//	so that it can find the appropriate device.

	else
		{
		//	Set the data

		if (!ItemList.SetPropertyAtCursor(this, sName, pValue, iCount, retsError))
			return false;

		//	Update the object

		ItemEnhancementModified(ItemList);
		}

	//	Done

	if (retItem)
		*retItem = ItemList.GetItemAtCursor();

	return true;
	}
