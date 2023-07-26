//
// Do NOT modify or remove this copyright and license
//
// Copyright (c) 2014-2022 Seagate Technology LLC and/or its Affiliates, All Rights Reserved
//
// This software is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// ******************************************************************************************
// \file EULA.h
// \brief This file defines the a function to print the EULA for the license option in a tool

#pragma once

#if defined (__cplusplus)
extern "C"
{
#endif

    #include <stdio.h>

    //-----------------------------------------------------------------------------
    //
    //  print_EULA_To_Screen(int showApacheLicense, int showZlibLicense)
    //
    //! \brief   Description:  Print the EULA to the screen
    //
    //  Entry:
    //!
    //  Exit:
    //
    //-----------------------------------------------------------------------------
    void print_EULA_To_Screen(void);

    //-----------------------------------------------------------------------------
    //
    //  print_Open_Source_Licenses(int showApacheLicense, int showZlibLicense)
    //
    //! \brief   Description:  Print the open source licenses to the screen
    //
    //  Entry:
    //!
    //  Exit:
    //
    //-----------------------------------------------------------------------------
    void print_Open_Source_Licenses(void);


#if defined (__cplusplus)
}
#endif
