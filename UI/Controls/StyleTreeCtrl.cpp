#include <StdAfx.h>
#include <UI/Controls/StyleTreeCtrl.h>
#include <UI/UIFunctions.h>

namespace controls
{
	static std::wstring CLASS = L"StyleTreeCtrl";

	void StyleTreeCtrl::Create(_In_ CWnd* pCreateParent, const UINT nIDContextMenu)
	{
		m_nIDContextMenu = nIDContextMenu;

		CTreeCtrl::Create(
			TVS_HASBUTTONS | TVS_LINESATROOT | TVS_EDITLABELS | TVS_DISABLEDRAGDROP | TVS_SHOWSELALWAYS |
				TVS_FULLROWSELECT | WS_CHILD | WS_TABSTOP | WS_CLIPSIBLINGS | WS_VISIBLE,
			CRect(0, 0, 0, 0),
			pCreateParent,
			IDC_FOLDER_TREE);
		TreeView_SetBkColor(m_hWnd, ui::MyGetSysColor(ui::cBackground));
		TreeView_SetTextColor(m_hWnd, ui::MyGetSysColor(ui::cText));
		::SendMessageA(m_hWnd, WM_SETFONT, reinterpret_cast<WPARAM>(ui::GetSegoeFont()), false);
	}

	BEGIN_MESSAGE_MAP(StyleTreeCtrl, CTreeCtrl)
	ON_NOTIFY_REFLECT(NM_CUSTOMDRAW, OnCustomDraw)
	END_MESSAGE_MAP()

	LRESULT StyleTreeCtrl::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
	{
		// Read the current hover local, since we need to clear it before we do any drawing
		const auto hItemCurHover = m_hItemCurHover;
		switch (message)
		{
		case WM_MOUSEMOVE:
		{
			TVHITTESTINFO tvHitTestInfo = {0};
			tvHitTestInfo.pt.x = GET_X_LPARAM(lParam);
			tvHitTestInfo.pt.y = GET_Y_LPARAM(lParam);

			WC_B_S(::SendMessage(m_hWnd, TVM_HITTEST, 0, reinterpret_cast<LPARAM>(&tvHitTestInfo)));
			if (tvHitTestInfo.hItem)
			{
				if (tvHitTestInfo.flags & TVHT_ONITEMBUTTON)
				{
					m_HoverButton = true;
				}
				else
				{
					m_HoverButton = false;
				}

				// If this is a new glow, clean up the old glow and track for leaving the control
				if (hItemCurHover != tvHitTestInfo.hItem)
				{
					ui::DrawTreeItemGlow(m_hWnd, tvHitTestInfo.hItem);

					if (hItemCurHover)
					{
						m_hItemCurHover = nullptr;
						ui::DrawTreeItemGlow(m_hWnd, hItemCurHover);
					}

					m_hItemCurHover = tvHitTestInfo.hItem;

					TRACKMOUSEEVENT tmEvent = {0};
					tmEvent.cbSize = sizeof(TRACKMOUSEEVENT);
					tmEvent.dwFlags = TME_LEAVE;
					tmEvent.hwndTrack = m_hWnd;

					WC_B_S(TrackMouseEvent(&tmEvent));
				}
			}
			else
			{
				if (hItemCurHover)
				{
					m_hItemCurHover = nullptr;
					ui::DrawTreeItemGlow(m_hWnd, hItemCurHover);
				}
			}
			break;
		}
		case WM_MOUSELEAVE:
			if (hItemCurHover)
			{
				m_hItemCurHover = nullptr;
				ui::DrawTreeItemGlow(m_hWnd, hItemCurHover);
			}

			return NULL;
		}

		return CTreeCtrl::WindowProc(message, wParam, lParam);
	}

	void StyleTreeCtrl::OnCustomDraw(_In_ NMHDR* pNMHDR, _In_ LRESULT* pResult)
	{
		ui::CustomDrawTree(pNMHDR, pResult, m_HoverButton, m_hItemCurHover);
	}

	// Removes any existing node data and replaces it with lpData
	void StyleTreeCtrl::SetNodeData(HWND hWnd, HTREEITEM hItem, const LPARAM lpData) const
	{
		if (lpData)
		{
			TVITEM tvItem = {0};
			tvItem.hItem = hItem;
			tvItem.mask = TVIF_PARAM;
			if (TreeView_GetItem(hWnd, &tvItem) && tvItem.lParam)
			{
				output::DebugPrintEx(DBGHierarchy, CLASS, L"SetNodeData", L"Node %p, replacing data\n", hItem);
				FreeNodeData(tvItem.lParam);
			}
			else
			{
				output::DebugPrintEx(DBGHierarchy, CLASS, L"SetNodeData", L"Node %p, first data\n", hItem);
			}

			tvItem.lParam = lpData;
			TreeView_SetItem(hWnd, &tvItem);
			// The tree now owns our lpData
		}
	}
} // namespace controls