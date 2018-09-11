//	UIHelpers.h
//
//	Classes and methods for generating appropriate UI.
//	Copyright (c) 2012 by Kronosaur Productions, LLC. All Rights Reserved.

#pragma once

class CItemDataAnimatron : public IAnimatron
	{
	public:
		struct SDamageAdjCell
			{
			DamageTypes iDamage;
			CString sText;
			};

		CItemDataAnimatron (const CVisualPalette &VI, const CItem &Item);

		inline bool IsEmpty (void) const { return (m_DamageAdj.GetCount() == 0); }

		//	IAnimatron virtuals
		virtual void GetSpacingRect (RECT *retrcRect);
		virtual void Paint (SAniPaintCtx &Ctx);

		static void CalcDamageAdj (const CItem &Item, TArray<SDamageAdjCell> *retDamageAdj);
		static int MeasureDamageAdj (const CG16bitFont &Font, const TArray<SDamageAdjCell> &DamageAdj);

	private:
		const CVisualPalette &m_VI;
		CItem m_Item;

		bool m_bIsWeapon;
		TArray<SDamageAdjCell> m_DamageAdj;
	};

class CListCollectionTask : public IHITask
	{
	public:
		CListCollectionTask (CHumanInterface &HI, 
							 CExtensionCollection &Extensions, 
							 CMultiverseModel &Multiverse, 
							 CCloudService &Service, 
							 int cxWidth, 
							 bool bNoCollectionRefresh = false,
							 bool bDebugMode = false);
		~CListCollectionTask (void);

		inline IAnimatron *GetListHandoff (void) { IAnimatron *pResult = m_pList; m_pList = NULL; return pResult; }

		//	IHITask virtuals
		virtual ALERROR OnExecute (ITaskProcessor *pProcessor, CString *retsResult);

	private:
		void CreateEntry (CMultiverseCatalogEntry *pCatalogEntry, int yStart, IAnimatron **retpEntry, int *retcyHeight);

		CExtensionCollection &m_Extensions;
		CMultiverseModel &m_Multiverse;
		CCloudService &m_Service;
		int m_cxWidth;
		bool m_bNoCollectionRefresh;
		bool m_bDebugMode;

		CAniListBox *m_pList;
	};

class CListSaveFilesTask : public IHITask
	{
	public:
		CListSaveFilesTask (CHumanInterface &HI, const TArray<CString> &Folders, const CString &sUsername, bool bFilterPermadeath, int cxWidth);
		~CListSaveFilesTask (void);

		inline IAnimatron *GetListHandoff (void) { IAnimatron *pResult = m_pList; m_pList = NULL; return pResult; }

		//	IHITask virtuals
		virtual ALERROR OnExecute (ITaskProcessor *pProcessor, CString *retsResult);

	private:
		void CreateFileEntry (CGameFile &GameFile, const CTimeDate &ModifiedTime, int yStart, IAnimatron **retpEntry, int *retcyHeight);

		TArray<CString> m_Folders;
		CString m_sUsername;
		bool m_bFilterPermadeath;
		int m_cxWidth;

		CAniListBox *m_pList;
	};

class CReadProfileTask : public IHITask
	{
	public:
		CReadProfileTask (CHumanInterface &HI, CCloudService &Service, int cxWidth);
		~CReadProfileTask (void);

		inline IAnimatron *GetListHandoff (void) { IAnimatron *pResult = m_pList; m_pList = NULL; return pResult; }

		//	IHITask virtuals
		virtual ALERROR OnExecute (ITaskProcessor *pProcessor, CString *retsResult);

	private:
		void CreateAdventureRecordDisplay (CAdventureRecord &Record, int yStart, IAnimatron **retpAni);

		CCloudService &m_Service;
		int m_cxWidth;

		CUserProfile m_Profile;

		CAniVScroller *m_pList;
	};

class CUIHelper
	{
	public:
		enum EOptions
			{
			//	CreateClassInfo???
			OPTION_ITEM_RIGHT_ALIGN =				0x00000001,

			//	CreateSessionFrameBar
			OPTION_FRAME_ALIGN_TOP =				0x00000001,
			OPTION_FRAME_ALIGN_BOTTOM =				0x00000002,

			//	CreateSessionTitle
			OPTION_SESSION_OK_BUTTON =				0x00000001,
			OPTION_SESSION_NO_CANCEL_BUTTON =		0x00000002,
			OPTION_SESSION_ADD_EXTENSION_BUTTON =	0x00000004,
			OPTION_SESSION_NO_HEADER =				0x00000008,

			//	PaintItemEntry
			OPTION_SELECTED =						0x00000001,
			OPTION_NO_ICON =						0x00000002,
			OPTION_TITLE =							0x00000004,
			OPTION_SMALL_ICON =						0x00000008,		//	Paint icon at 64x64
			OPTION_NO_PADDING =						0x00000010,		//	No padding around item
			OPTION_NO_ARMOR_SPEED_DISPLAY =			0x00000020,		//	Do not show bonus/penalty for armor
			OPTION_KNOWN =							0x00000040,		//	Show items as known

			//	SMenuEntry flags
			MENU_TEXT =								0x00000001,		//	Show on left-hand text menu
			MENU_ALIGN_CENTER =						0x00000002,		//	Align center
			MENU_ALIGN_LEFT =						0x00000004,		//	Align left
			MENU_ALIGN_RIGHT =						0x00000008,		//	Align right
			};

		struct SMenuEntry
			{
			SMenuEntry (void) : 
					pIcon(NULL),
					dwFlags(0)
				{ }

			CString sLabel;
			CString sCommand;
			const CG32bitImage *pIcon;
			DWORD dwFlags;
			};

		CUIHelper (CHumanInterface &HI) : m_HI(HI) { }

		int CalcItemEntryHeight (CSpaceObject *pSource, const CItem &Item, const RECT &rcRect, DWORD dwOptions) const;

		void CreateClassInfoArmor (CShipClass *pClass, int x, int y, int cxWidth, DWORD dwOptions, int *retcyHeight, IAnimatron **retpInfo) const;
		void CreateClassInfoCargo (CShipClass *pClass, const CDeviceDescList &Devices, int x, int y, int cxWidth, DWORD dwOptions, int *retcyHeight, IAnimatron **retpInfo) const;
		void CreateClassInfoDeviceSlots (CShipClass *pClass, const CDeviceDescList &Devices, int x, int y, int cxWidth, DWORD dwOptions, int *retcyHeight, IAnimatron **retpInfo) const;
		void CreateClassInfoDrive (CShipClass *pClass, const CDeviceDescList &Devices, int x, int y, int cxWidth, DWORD dwOptions, int *retcyHeight, IAnimatron **retpInfo) const;
		void CreateClassInfoItem (const CItem &Item, int x, int y, int cxWidth, DWORD dwOptions, const CString &sExtraDesc, int *retcyHeight, IAnimatron **retpInfo) const;
		void CreateClassInfoReactor (CShipClass *pClass, const CDeviceDescList &Devices, int x, int y, int cxWidth, DWORD dwOptions, int *retcyHeight, IAnimatron **retpInfo) const;
		void CreateInputErrorMessage (IHISession *pSession, const RECT &rcRect, const CString &sTitle, CString &sDesc, IAnimatron **retpMsg = NULL) const;
		void CreateSessionFrameBar (IHISession *pSession, const TArray<SMenuEntry> *pMenu, DWORD dwOptions, IAnimatron **retpControl) const;
		void CreateSessionTitle (IHISession *pSession, 
								 CCloudService &Service, 
								 const CString &sTitle, 
								 const TArray<SMenuEntry> *pMenu,
								 DWORD dwOptions, 
								 IAnimatron **retpControl) const;
		void CreateSessionWaitAnimation (const CString &sID, const CString &sText, IAnimatron **retpControl) const;

		void FormatDisplayAttributes (TArray<SDisplayAttribute> &Attribs, const RECT &rcRect, int *retcyHeight) const;

		void GenerateDockScreenRTF (const CString &sText, CString *retsRTF) const;

		void PaintDisplayAttributes (CG32bitImage &Dest, TArray<SDisplayAttribute> &Attribs) const;
		void PaintItemEntry (CG32bitImage &Dest, CSpaceObject *pSource, const CItem &Item, const RECT &rcRect, CG32bitPixel rgbText, DWORD dwOptions) const;
		void PaintReferenceDamageAdj (CG32bitImage &Dest, int x, int y, int iLevel, int iHP, const int *iDamageAdj, CG32bitPixel rgbText) const;
		void PaintReferenceDamageType (CG32bitImage &Dest, int x, int y, int iDamageType, const CString &sRef, CG32bitPixel rgbText) const;

		static int ScrollAnimationDecay (int iOffset);

	private:
		void CreateClassInfoSpecialItem (CItemType *pItemIcon, const CString &sText, int x, int y, int cxWidth, DWORD dwOptions, int *retcyHeight, IAnimatron **retpInfo) const;
		void CreateBarButtons (CAniSequencer *pSeq, const RECT &rcRect, IHISession *pSession, const TArray<SMenuEntry> *pMenu, DWORD dwOptions) const;

		CHumanInterface &m_HI;
	};

class CInputErrorMessageController : public IAnimatron, public IAniCommand
	{
	public:
		CInputErrorMessageController (IHISession *pSession) : m_pSession(pSession) { }

		//	IAnimatron virtuals
		virtual void GetSpacingRect (RECT *retrcRect) { retrcRect->left = 0; retrcRect->right = 0; retrcRect->top = 0; retrcRect->bottom = 0; }
		virtual void Paint (SAniPaintCtx &Ctx) { }

	protected:
		//	IAniCommand virtuals
		virtual void OnAniCommand (const CString &sID, const CString &sEvent, const CString &sCmd, DWORD dwData);

	private:
		IHISession *m_pSession;
	};
