/*
 * Copyright (c) 2015 Martin Mitas
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2.1 of the License, or
 * (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "dwm.h"

#include <dwmapi.h>


static HMODULE dwm_dll;

static HRESULT (WINAPI* dwm_DwmIsCompositionEnabled)(BOOL*);
static HRESULT (WINAPI* dwm_DwmExtendFrameIntoClientArea)(HWND, const MARGINS*);


BOOL
dwm_is_composition_enabled(void)
{
    HRESULT hr;
    BOOL enabled;

    if(dwm_DwmIsCompositionEnabled == NULL)
        return FALSE;

    hr = dwm_DwmIsCompositionEnabled(&enabled);
    if(MC_ERR(FAILED(hr))) {
        MC_TRACE("dwm_is_composition_enabled: DwmIsCompositionEnabled(). [0x%lx]", hr);
        return FALSE;
    }

    return enabled;
}

void
dwm_extend_frame(HWND win, int margin_left, int margin_top,
                           int margin_right, int margin_bottom)
{
    MARGINS margins = { margin_left, margin_right, margin_top, margin_bottom };

    MC_ASSERT(dwm_DwmExtendFrameIntoClientArea != NULL);
    dwm_DwmExtendFrameIntoClientArea(win, &margins);
}


int
dwm_init_module(void)
{
    dwm_dll = mc_load_sys_dll(_T("DWMAPI.DLL"));
    if(dwm_dll == NULL)
        goto done;

    dwm_DwmIsCompositionEnabled = (HRESULT (WINAPI*)(BOOL*))
                GetProcAddress(dwm_dll, "DwmIsCompositionEnabled");
    dwm_DwmExtendFrameIntoClientArea = (HRESULT (WINAPI*)(HWND, const MARGINS*))
                GetProcAddress(dwm_dll, "DwmExtendFrameIntoClientArea");
    if(dwm_DwmIsCompositionEnabled == NULL  ||  dwm_DwmExtendFrameIntoClientArea == NULL)
        goto no_dwm;

    /* Success. */
    return 0;

    /* "Error" path. */
no_dwm:
    dwm_DwmIsCompositionEnabled = NULL;
    FreeLibrary(dwm_dll);
    dwm_dll = NULL;
done:
    return 0;
}

void
dwm_fini_module(void)
{
    if(dwm_dll != NULL)
        FreeLibrary(dwm_dll);
}
