//
// Do NOT modify or remove this copyright and license
//
// Copyright (c) 2014-2018 Seagate Technology LLC and/or its Affiliates, All Rights Reserved
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

    //-----------------------------------------------------------------------------
    //
    //  print_GNU_LGPL_License()
    //
    //! \brief   Description:  Print the GNU LGPL license to the screen
    //
    //  Entry:
    //!
    //  Exit:
    //!   \return VOID
    //
    //-----------------------------------------------------------------------------
    void print_GNU_LGPL_License();

    //-----------------------------------------------------------------------------
    //
    //  print_FreeBSD_License()
    //
    //! \brief   Description:  Print the FreeBSD 2 clause BSD license to the screen
    //
    //  Entry:
    //!
    //  Exit:
    //!   \return VOID
    //
    //-----------------------------------------------------------------------------
    void print_FreeBSD_License();

    //-----------------------------------------------------------------------------
    //
    //  print_Apache_2_0_License()
    //
    //! \brief   Description:  Print the Apache 2.0 license to the screen (for mbedtls)
    //
    //  Entry:
    //!
    //  Exit:
    //!   \return VOID
    //
    //-----------------------------------------------------------------------------
    void print_Apache_2_0_License();

    //-----------------------------------------------------------------------------
    //
    //  print_Zlib_License()
    //
    //! \brief   Description:  Print the ZLib license to the screen
    //
    //  Entry:
    //!
    //  Exit:
    //!   \return VOID
    //
    //-----------------------------------------------------------------------------
    void print_Zlib_License();

    //-----------------------------------------------------------------------------
    //
    //  print_Win_Getopt_Licenses()
    //
    //! \brief   Description:  Print the Win Getopt project license to the screen
    //
    //  Entry:
    //!
    //  Exit:
    //!   \return VOID
    //
    //-----------------------------------------------------------------------------
    void print_Win_Getopt_Licenses();

#if defined (__cplusplus)
}
#endif