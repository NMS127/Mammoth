//	CListCollectionTask.cpp
//
//	CListCollectionTask class
//	Copyright (c) 2013 by Kronosaur Productions, LLC. All Rights Reserved.

#include "stdafx.h"

#define ALIGN_CENTER							CONSTLIT("center")
#define ALIGN_RIGHT								CONSTLIT("right")

#define FILL_TYPE_NONE							CONSTLIT("none")

#define LINE_TYPE_SOLID							CONSTLIT("solid")

#define PROP_COLOR								CONSTLIT("color")
#define PROP_ENABLED							CONSTLIT("enabled")
#define PROP_FADE_EDGE_HEIGHT					CONSTLIT("fadeEdgeHeight")
#define PROP_FILL_TYPE							CONSTLIT("fillType")
#define PROP_FONT								CONSTLIT("font")
#define PROP_LINE_COLOR							CONSTLIT("lineColor")
#define PROP_LINE_PADDING						CONSTLIT("linePadding")
#define PROP_LINE_TYPE							CONSTLIT("lineType")
#define PROP_LINE_WIDTH							CONSTLIT("lineWidth")
#define PROP_LL_RADIUS							CONSTLIT("llRadius")
#define PROP_LR_RADIUS							CONSTLIT("lrRadius")
#define PROP_OPACITY							CONSTLIT("opacity")
#define PROP_PADDING_BOTTOM						CONSTLIT("paddingBottom")
#define PROP_POSITION							CONSTLIT("position")
#define PROP_SCALE								CONSTLIT("scale")
#define PROP_TEXT								CONSTLIT("text")
#define PROP_TEXT_ALIGN_HORZ					CONSTLIT("textAlignHorz")
#define PROP_UL_RADIUS							CONSTLIT("ulRadius")
#define PROP_UR_RADIUS							CONSTLIT("urRadius")
#define PROP_VIEWPORT_HEIGHT					CONSTLIT("viewportHeight")

#define STYLE_SELECTION							CONSTLIT("selection")
#define STYLE_SELECTION_FOCUS					CONSTLIT("selectionFocus")

#define ERR_GET_COLLECTION_FAILED				CONSTLIT("Unable to get collection.")

const int ENTRY_ICON_HEIGHT =					150;
const int ENTRY_ICON_WIDTH =					300;

const int ICON_SPACING_HORZ =					16;
const int INTER_LINE_SPACING =					20;
const int MAJOR_PADDING_BOTTOM =				20;
const int MAJOR_PADDING_TOP =					20;
const int TYPE_CORNER_RADIUS =					3;
const int ICON_RADIUS =							6;

const int SELECTION_BORDER_WIDTH =				1;
const int SELECTION_CORNER_RADIUS =				8;

CListCollectionTask::CListCollectionTask (CHumanInterface &HI, 
										  CExtensionCollection &Extensions, 
										  CMultiverseModel &Multiverse, 
										  CCloudService &Service, 
										  const SOptions &Options) : IHITask(HI), 
		m_Extensions(Extensions),
		m_Multiverse(Multiverse),
		m_Service(Service),
		m_Options(Options)

//	CListCollectionTask constructor

	{
	}

CListCollectionTask::~CListCollectionTask (void)

//	CListCollectionTask destructor

	{
	if (m_pList)
		delete m_pList;
	}

void CListCollectionTask::CreateEntry (CMultiverseCatalogEntry *pCatalogEntry, int yStart, IAnimatron **retpEntry, int *retcyHeight)

//	CreateEntry
//
//	Creates a new catalog entry

	{
	const CVisualPalette &VI = m_HI.GetVisuals();
	const CG16bitFont &MediumFont = VI.GetFont(fontMedium);
	const CG16bitFont &TypeFont = VI.GetFont(fontMediumHeavyBold);
	const CG16bitFont &SubTitleFont = VI.GetFont(fontSubTitle);

	int x = 0;
	int y = 0;
	int xText = x + ENTRY_ICON_WIDTH + ICON_SPACING_HORZ;
	int cxText = RectWidth(m_Options.rcRect) - (ENTRY_ICON_WIDTH + ICON_SPACING_HORZ);

	//	Start with a sequencer

	CAniSequencer *pRoot = new CAniSequencer;
	pRoot->SetPropertyVector(PROP_POSITION, CVector(0, yStart));

	//	Add the icon

	CG32bitImage *pIcon = CreateEntryIcon(*pCatalogEntry);
	if (pIcon)
		{
		int xOffset = (ENTRY_ICON_WIDTH - pIcon->GetWidth()) / 2;
		IAnimatron *pIconAni = new CAniRoundedRect;
		pIconAni->SetPropertyVector(PROP_POSITION, CVector(x + xOffset, 0));
		pIconAni->SetPropertyVector(PROP_SCALE, CVector(pIcon->GetWidth(), pIcon->GetHeight()));
		pIconAni->SetPropertyInteger(PROP_UL_RADIUS, ICON_RADIUS);
		pIconAni->SetPropertyInteger(PROP_UR_RADIUS, ICON_RADIUS);
		pIconAni->SetPropertyInteger(PROP_LL_RADIUS, ICON_RADIUS);
		pIconAni->SetPropertyInteger(PROP_LR_RADIUS, ICON_RADIUS);
		pIconAni->SetFillMethod(new CAniImageFill(pIcon, true));

		pRoot->AddTrack(pIconAni, 0);
		}

	//	Add the extension name

	CString sHeading = pCatalogEntry->GetName();

	IAnimatron *pName = new CAniText;
	pName->SetPropertyVector(PROP_POSITION, CVector(xText, y));
	pName->SetPropertyVector(PROP_SCALE, CVector(10000, 1000));
	pName->SetPropertyColor(PROP_COLOR, VI.GetColor(colorTextDialogInput));
	pName->SetPropertyFont(PROP_FONT, &SubTitleFont);
	pName->SetPropertyString(PROP_TEXT, sHeading);

	pRoot->AddTrack(pName, 0);
	y += SubTitleFont.GetHeight();

	//	Add the type of extension

	CString sType;
	switch (pCatalogEntry->GetType())
		{
		case extAdventure:
			sType = CONSTLIT("Adventure");
			break;

		case extLibrary:
			sType = CONSTLIT("Library");
			break;

		default:
			sType = CONSTLIT("Expansion");
		}

	int cxTypeBackground = TypeFont.MeasureText(sType) + 2 * TYPE_CORNER_RADIUS;
	int cyTypeBackground = TypeFont.GetHeight() + 2 * TYPE_CORNER_RADIUS;

	IAnimatron *pTypeBackground = new CAniRoundedRect;
	pTypeBackground->SetPropertyColor(PROP_COLOR, VI.GetColor(colorTextNormal));
	pTypeBackground->SetPropertyOpacity(PROP_OPACITY, 255);
	pTypeBackground->SetPropertyInteger(PROP_UL_RADIUS, TYPE_CORNER_RADIUS);
	pTypeBackground->SetPropertyInteger(PROP_UR_RADIUS, TYPE_CORNER_RADIUS);
	pTypeBackground->SetPropertyInteger(PROP_LL_RADIUS, TYPE_CORNER_RADIUS);
	pTypeBackground->SetPropertyInteger(PROP_LR_RADIUS, TYPE_CORNER_RADIUS);
	pTypeBackground->SetPropertyVector(PROP_POSITION, CVector(xText, y + TYPE_CORNER_RADIUS));
	pTypeBackground->SetPropertyVector(PROP_SCALE, CVector(cxTypeBackground, TypeFont.GetHeight()));
	pRoot->AddTrack(pTypeBackground, 0);

	IAnimatron *pTypeLabel = new CAniText;
	pTypeLabel->SetPropertyVector(PROP_POSITION, CVector(xText + TYPE_CORNER_RADIUS, y + TYPE_CORNER_RADIUS));
	pTypeLabel->SetPropertyVector(PROP_SCALE, CVector(10000, 1000));
	pTypeLabel->SetPropertyColor(PROP_COLOR, VI.GetColor(colorAreaDeep));
	pTypeLabel->SetPropertyFont(PROP_FONT, &TypeFont);
	pTypeLabel->SetPropertyString(PROP_TEXT, sType);

	pRoot->AddTrack(pTypeLabel, 0);

	//	Add a version string

	CString sVersion;
	if (!pCatalogEntry->GetVersion().IsBlank())
		sVersion = strPatternSubst(CONSTLIT("version: %s"), pCatalogEntry->GetVersion());

	//	Add the version number

	if (!sVersion.IsBlank())
		{
		IAnimatron *pVersion = new CAniText;
		pVersion->SetPropertyVector(PROP_POSITION, CVector(xText + cxTypeBackground + ICON_SPACING_HORZ, y + TYPE_CORNER_RADIUS));
		pVersion->SetPropertyVector(PROP_SCALE, CVector(10000, 1000));
		pVersion->SetPropertyColor(PROP_COLOR, VI.GetColor(colorTextDialogLabel));
		pVersion->SetPropertyFont(PROP_FONT, &MediumFont);
		pVersion->SetPropertyString(PROP_TEXT, sVersion);

		pRoot->AddTrack(pVersion, 0);
		}

	y += cyTypeBackground;

	//	Add status, if necessary

	CString sStatus;
	CG32bitPixel rgbStatus;
	switch (pCatalogEntry->GetStatus())
		{
		case CMultiverseCatalogEntry::statusCorrupt:
			sStatus = CONSTLIT("Registration signature does not match.");
			rgbStatus = VI.GetColor(colorTextDialogWarning);
			break;

		case CMultiverseCatalogEntry::statusError:
			sStatus = pCatalogEntry->GetStatusText();
			rgbStatus = VI.GetColor(colorTextDialogWarning);
			break;

		case CMultiverseCatalogEntry::statusPlayerDisabled:
			sStatus = CONSTLIT("Disabled");
			rgbStatus = VI.GetColor(colorTextNormal);
			break;

		case CMultiverseCatalogEntry::statusNotAvailable:
			sStatus = CONSTLIT("Not loaded");
			rgbStatus = VI.GetColor(colorTextNormal);
			break;
		}

	if (!sStatus.IsBlank())
		{
		IAnimatron *pStatus = new CAniText;
		pStatus->SetPropertyVector(PROP_POSITION, CVector(xText, y));
		pStatus->SetPropertyVector(PROP_SCALE, CVector(cxText, 1000));
		pStatus->SetPropertyColor(PROP_COLOR, rgbStatus);
		pStatus->SetPropertyFont(PROP_FONT, &MediumFont);
		pStatus->SetPropertyString(PROP_TEXT, sStatus);

		RECT rcLine;
		pStatus->GetSpacingRect(&rcLine);

		pRoot->AddTrack(pStatus, 0);
		y += RectHeight(rcLine);
		}

	//	Now add the description

	CString sDesc = pCatalogEntry->GetDesc();

	IAnimatron *pDesc = new CAniText;
	pDesc->SetPropertyVector(PROP_POSITION, CVector(xText, y));
	pDesc->SetPropertyVector(PROP_SCALE, CVector(cxText, 1000));
	pDesc->SetPropertyColor(PROP_COLOR, VI.GetColor(colorTextDialogInput));
	pDesc->SetPropertyFont(PROP_FONT, &MediumFont);
	pDesc->SetPropertyString(PROP_TEXT, sDesc);

	RECT rcLine;
	pDesc->GetSpacingRect(&rcLine);

	pRoot->AddTrack(pDesc, 0);
	y += RectHeight(rcLine);

	//	Done

	*retpEntry = pRoot;

	if (retcyHeight)
		*retcyHeight = Max(ENTRY_ICON_HEIGHT, y);
	}

CG32bitImage *CListCollectionTask::CreateEntryIcon (CMultiverseCatalogEntry &Entry) const

//	CreateEntryIcon
//
//	Creates an appropriate icon for the given entry. Caller is responsible for
//	freeing the result.

	{
	//	Get the icon for the extension or the generic icon.

	CG32bitImage *pIcon = Entry.GetIcon();
	if (pIcon == NULL)
		pIcon = m_Options.pGenericIcon;
	if (pIcon == NULL)
		return NULL;

	//	In some cases we need to gray out the icon

	switch (Entry.GetStatus())
		{
		//	For disabled, etc., we gray out the icon

		case CMultiverseCatalogEntry::statusCorrupt:
		case CMultiverseCatalogEntry::statusDownloadInProgress:
		case CMultiverseCatalogEntry::statusError:
		case CMultiverseCatalogEntry::statusNotAvailable:
		case CMultiverseCatalogEntry::statusPlayerDisabled:
		case CMultiverseCatalogEntry::statusUnknown:
			{
			CG32bitImage *pNewIcon = new CG32bitImage;
			pNewIcon->Create(pIcon->GetWidth(), pIcon->GetHeight(), pIcon->GetAlphaType());
			CGDraw::BltGray(*pNewIcon, 0, 0, *pIcon, 0, 0, pIcon->GetWidth(), pIcon->GetHeight(), 0x80);

			return pNewIcon;
			}

		//	Otherwise, normal icon

		default:
			return new CG32bitImage(*pIcon);
		}
	}

ALERROR CListCollectionTask::OnExecute (ITaskProcessor *pProcessor, CString *retsResult)

//	OnExecute
//
//	Execute the task
	
	{
	int i;

	const CVisualPalette &VI = m_HI.GetVisuals();

	//	Get the list of entries from the Multiverse and update the status from 
	//	the extensions.

	CExtensionCollection::SCollectionStatusOptions Options;
	Options.cxIconSize = ENTRY_ICON_WIDTH;
	Options.cyIconSize = ENTRY_ICON_HEIGHT;

	m_Collection = m_Multiverse.GetCollection();
	m_Extensions.UpdateCollectionStatus(m_Collection, Options);

	//	Sort the entries

	TSortMap<CString, CMultiverseCatalogEntry *> SortedList;
	for (i = 0; i < m_Collection.GetCount(); i++)
		{
		CMultiverseCatalogEntry *pEntry = &m_Collection[i];

		//	Skip libraries unless in debug mode

		if (!m_Options.bShowLibraries && pEntry->GetType() == extLibrary)
			continue;

		//	Sort key

		CString sDomain;
		if (IsOfficialUNID(pEntry->GetUNID()))
			sDomain = CONSTLIT("01");
		else if (IsRegisteredUNID(pEntry->GetUNID()))
			sDomain = CONSTLIT("02");
		else
			sDomain = CONSTLIT("03");

		CString sType;
		if (pEntry->GetType() == extAdventure)
			sType = CONSTLIT("01");
		else if (pEntry->GetType() == extExtension)
			sType = CONSTLIT("02");
		else
			sType = CONSTLIT("03");

		CString sSortKey = strPatternSubst(CONSTLIT("%s:%s:%s"), sDomain, sType, pEntry->GetName());

		//	Add

		SortedList.Insert(sSortKey, pEntry);
		}

	//	Generate a Reanimator list of all entries. The root will be a CAniListBox

    VI.CreateListBox(NULL, NULL_STR, 0, 0, 0, 0, 0, &m_pList);

	//	No need to log image load

	g_pUniverse->SetLogImageLoad(false);

	//	Loop over all entries in the collection and add them to the list

	CString sSelection;
	int y = MAJOR_PADDING_TOP;
	for (i = 0; i < SortedList.GetCount(); i++)
		{
		CMultiverseCatalogEntry *pCatalogEntry = SortedList[i];

		//	Generate a record for the file

		IAnimatron *pEntry;
		int cyHeight;
		CreateEntry(pCatalogEntry, y, &pEntry, &cyHeight);

		CString sID = strFromInt(pCatalogEntry->GetUNID());
		m_pList->AddEntry(sID, pEntry);
		y += cyHeight + INTER_LINE_SPACING;

		//	If we want to select this entry, then remember the index

		if (m_Options.dwSelectUNID && pCatalogEntry->GetUNID() == m_Options.dwSelectUNID)
			sSelection = sID;
		}

	//	Position the list

	m_pList->SetPropertyVector(PROP_POSITION, CVector(m_Options.rcRect.left, m_Options.rcRect.top));
	m_pList->SetPropertyVector(PROP_SCALE, CVector(RectWidth(m_Options.rcRect), RectHeight(m_Options.rcRect)));
	m_pList->SetPropertyMetric(PROP_VIEWPORT_HEIGHT, (Metric)RectHeight(m_Options.rcRect));

	//	Select

	if (!sSelection.IsBlank())
		m_pList->SetSelection(sSelection);

	g_pUniverse->SetLogImageLoad(true);

	//	Done

	return NOERROR;
	}
