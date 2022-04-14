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
    //!   \param showApacheLicense = set to non-zero value to print out the Apache 2.0 license (needed if using mbedtls/encryption - fwdl config file)
    //!   \param showZlibLicesne = set to non-zero value to print out the ZLib license (needed if using zlib/compression - fwdl config file)
    //!
    //  Exit:
    //!   \return VOID
    //
    //-----------------------------------------------------------------------------
    void print_EULA_To_Screen(int showApacheLicense, int showZlibLicense);

    //-----------------------------------------------------------------------------
    //
    //  print_Open_Source_Licenses(int showApacheLicense, int showZlibLicense)
    //
    //! \brief   Description:  Print the open source licenses to the screen
    //
    //  Entry:
    //!   \param showApacheLicense = set to non-zero value to print out the Apache 2.0 license (needed if using mbedtls/encryption - fwdl config file)
    //!   \param showZlibLicesne = set to non-zero value to print out the ZLib license (needed if using zlib/compression - fwdl config file)
    //!
    //  Exit:
    //!   \return VOID
    //
    //-----------------------------------------------------------------------------
    void print_Open_Source_Licenses(int showApacheLicense, int showZlibLicense);


#if defined (__cplusplus)
}
#endif
