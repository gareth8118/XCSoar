/*
Copyright_License {

  XCSoar Glide Computer - http://www.xcsoar.org/
  Copyright (C) 2000-2011 The XCSoar Project
  A detailed list of copyright holders can be found in the file "AUTHORS".

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; either version 2
  of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
}
*/

#include "Dialogs/Internal.hpp"
#include "Language/Language.hpp"

#if defined(WIN32) && !defined(_WIN32_WCE) && !defined(HAVE_POSIX)

#include "Weather/NOAAStore.hpp"
#include "Weather/METAR.hpp"
#include "Weather/TAF.hpp"
#include "Screen/Layout.hpp"

static WndForm *wf = NULL;
static unsigned index;

static void
Update()
{
  tstring metar_taf;

  METAR metar;
  if (!NOAAStore::GetMETAR(index, metar)) {
    metar_taf = _("No METAR available!");
  } else {
    metar_taf = metar.content.c_str();
  }

  metar_taf += _T("\n\n");

  TAF taf;
  if (!NOAAStore::GetTAF(index, taf)) {
    metar_taf += _("No TAF available!");
  } else {
    metar_taf += taf.content.c_str();
  }

  WndProperty* wp = (WndProperty*)wf->FindByName(_T("DetailsText"));
  wp->SetText(metar_taf.c_str());
}

static void
UpdateClicked(gcc_unused WndButton &Sender)
{
  NOAAStore::UpdateStation(index);
  Update();
}

static void
RemoveClicked(gcc_unused WndButton &Sender)
{
  TCHAR tmp[256];
  _stprintf(tmp, _("Do you want to remove station %s?"),
            NOAAStore::GetCodeT(index));

  if (MessageBoxX(tmp, _("Remove"), MB_YESNO) == IDNO)
    return;

  NOAAStore::RemoveStation(index);
  wf->SetModalResult(mrOK);
}

static void
CloseClicked(gcc_unused WndButton &Sender)
{
  wf->SetModalResult(mrOK);
}

static CallBackTableEntry CallBackTable[] = {
  DeclareCallBackEntry(UpdateClicked),
  DeclareCallBackEntry(RemoveClicked),
  DeclareCallBackEntry(CloseClicked),
  DeclareCallBackEntry(NULL)
};

void
dlgNOAADetailsShowModal(SingleWindow &parent, unsigned _index)
{
  assert(_index < NOAAStore::Count());
  index = _index;

  wf = LoadDialog(CallBackTable, parent, Layout::landscape ?
                  _T("IDR_XML_NOAA_DETAILS_L") : _T("IDR_XML_NOAA_DETAILS"));
  assert(wf != NULL);

  TCHAR caption[100];
  _stprintf(caption, _T("%s: %s"), _("METAR and TAF"),
            NOAAStore::GetCodeT(index));
  wf->SetCaption(caption);

  Update();

  wf->ShowModal();

  delete wf;
}

#else

#include "Dialogs/Message.hpp"

void
dlgNOAADetailsShowModal(SingleWindow &parent, unsigned index)
{
  MessageBoxX(_("This function is not available on your platform yet."),
              _("Error"), MB_OK);
}
#endif
