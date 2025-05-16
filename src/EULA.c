// SPDX-License-Identifier: MPL-2.0
//
// Do NOT modify or remove this copyright and license
//
// Copyright (c) 2014-2025 Seagate Technology LLC and/or its Affiliates, All Rights Reserved
//
// This software is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// ******************************************************************************************
//

// \file EULA.c
// \brief This file defines the a function to print the EULA for the license option in a tool
// BR note: http://tomeko.net/online_tools/cpp_text_escape.php?lang=en
// BR note: added print_wingetopt_EULA_To_Screen, print_mbedtls_zlib_EULA_To_Screen functions (22-Aug-2016)
// BR note: added Modifications notice to mgedtls license function (23-Aug-2016)
// BR note: openSeaChest project released under Mozilla Public License Version 2.0 (01-Nov-2017)

#include "EULA.h"
#include "predef_env_detect.h"

#include <stdio.h>

void print_EULA_To_Screen(void)
{
    fputs("BINARIES and SOURCE CODE files of the openSeaChest open source project have\n", stdout);
    fputs("been made available to you under the Mozilla Public License 2.0 (MPL).  Mozilla\n", stdout);
    fputs("is the custodian of the Mozilla Public License (\"MPL\"), an open source/free\n", stdout);
    fputs("software license.\n\n", stdout);

    fputs("https://www.mozilla.org/en-US/MPL/\n\n", stdout);

    fputs("Mozilla Public License Version 2.0\n", stdout);
    fputs("==================================\n\n", stdout);

    fputs("1. Definitions\n", stdout);
    fputs("--------------\n\n", stdout);

    fputs("1.1. \"Contributor\"\n", stdout);
    fputs("    means each individual or legal entity that creates, contributes to\n", stdout);
    fputs("    the creation of, or owns Covered Software.\n\n", stdout);

    fputs("1.2. \"Contributor Version\"\n", stdout);
    fputs("    means the combination of the Contributions of others (if any) used\n", stdout);
    fputs("    by a Contributor and that particular Contributor's Contribution.\n\n", stdout);

    fputs("1.3. \"Contribution\"\n", stdout);
    fputs("    means Covered Software of a particular Contributor.\n\n", stdout);

    fputs("1.4. \"Covered Software\"\n", stdout);
    fputs("    means Source Code Form to which the initial Contributor has attached\n", stdout);
    fputs("    the notice in Exhibit A, the Executable Form of such Source Code\n", stdout);
    fputs("    Form, and Modifications of such Source Code Form, in each case\n", stdout);
    fputs("    including portions thereof.\n\n", stdout);

    fputs("1.5. \"Incompatible With Secondary Licenses\"\n", stdout);
    fputs("    means\n\n", stdout);

    fputs("    (a) that the initial Contributor has attached the notice described\n", stdout);
    fputs("        in Exhibit B to the Covered Software; or\n\n", stdout);

    fputs("    (b) that the Covered Software was made available under the terms of\n", stdout);
    fputs("        version 1.1 or earlier of the License, but not also under the\n", stdout);
    fputs("        terms of a Secondary License.\n\n", stdout);

    fputs("1.6. \"Executable Form\"\n", stdout);
    fputs("    means any form of the work other than Source Code Form.\n\n", stdout);

    fputs("1.7. \"Larger Work\"\n", stdout);
    fputs("    means a work that combines Covered Software with other material, in\n", stdout);
    fputs("    a separate file or files, that is not Covered Software.\n\n", stdout);

    fputs("1.8. \"License\"\n", stdout);
    fputs("    means this document.\n\n", stdout);

    fputs("1.9. \"Licensable\"\n", stdout);
    fputs("    means having the right to grant, to the maximum extent possible,\n", stdout);
    fputs("    whether at the time of the initial grant or subsequently, any and\n", stdout);
    fputs("    all of the rights conveyed by this License.\n\n", stdout);

    fputs("1.10. \"Modifications\"\n", stdout);
    fputs("    means any of the following:\n\n", stdout);

    fputs("    (a) any file in Source Code Form that results from an addition to,\n", stdout);
    fputs("        deletion from, or modification of the contents of Covered\n", stdout);
    fputs("        Software; or\n\n", stdout);

    fputs("    (b) any new file in Source Code Form that contains any Covered\n", stdout);
    fputs("        Software.\n\n", stdout);

    fputs("1.11. \"Patent Claims\" of a Contributor\n", stdout);
    fputs("    means any patent claim(s), including without limitation, method,\n", stdout);
    fputs("    process, and apparatus claims, in any patent Licensable by such\n", stdout);
    fputs("    Contributor that would be infringed, but for the grant of the\n", stdout);
    fputs("    License, by the making, using, selling, offering for sale, having\n", stdout);
    fputs("    made, import, or transfer of either its Contributions or its\n", stdout);
    fputs("    Contributor Version.\n\n", stdout);

    fputs("1.12. \"Secondary License\"\n", stdout);
    fputs("    means either the GNU General Public License, Version 2.0, the GNU\n", stdout);
    fputs("    Lesser General Public License, Version 2.1, the GNU Affero General\n", stdout);
    fputs("    Public License, Version 3.0, or any later versions of those\n", stdout);
    fputs("    licenses.\n\n", stdout);

    fputs("1.13. \"Source Code Form\"\n", stdout);
    fputs("    means the form of the work preferred for making modifications.\n\n", stdout);

    fputs("1.14. \"You\" (or \"Your\")\n", stdout);
    fputs("    means an individual or a legal entity exercising rights under this\n", stdout);
    fputs("    License. For legal entities, \"You\" includes any entity that\n", stdout);
    fputs("    controls, is controlled by, or is under common control with You. For\n", stdout);
    fputs("    purposes of this definition, \"control\" means (a) the power, direct\n", stdout);
    fputs("    or indirect, to cause the direction or management of such entity,\n", stdout);
    fputs("    whether by contract or otherwise, or (b) ownership of more than\n", stdout);
    fputs("    fifty percent (50%) of the outstanding shares or beneficial\n", stdout);
    fputs("    ownership of such entity.\n\n", stdout);

    fputs("2. License Grants and Conditions\n", stdout);
    fputs("--------------------------------\n\n", stdout);

    fputs("2.1. Grants\n\n", stdout);

    fputs("Each Contributor hereby grants You a world-wide, royalty-free,\n", stdout);
    fputs("non-exclusive license:\n\n", stdout);

    fputs("(a) under intellectual property rights (other than patent or trademark)\n", stdout);
    fputs("    Licensable by such Contributor to use, reproduce, make available,\n", stdout);
    fputs("    modify, display, perform, distribute, and otherwise exploit its\n", stdout);
    fputs("    Contributions, either on an unmodified basis, with Modifications, or\n", stdout);
    fputs("    as part of a Larger Work; and\n\n", stdout);

    fputs("(b) under Patent Claims of such Contributor to make, use, sell, offer\n", stdout);
    fputs("    for sale, have made, import, and otherwise transfer either its\n", stdout);
    fputs("    Contributions or its Contributor Version.\n\n", stdout);

    fputs("2.2. Effective Date\n\n", stdout);

    fputs("The licenses granted in Section 2.1 with respect to any Contribution\n", stdout);
    fputs("become effective for each Contribution on the date the Contributor first\n", stdout);
    fputs("distributes such Contribution.\n\n", stdout);

    fputs("2.3. Limitations on Grant Scope\n\n", stdout);

    fputs("The licenses granted in this Section 2 are the only rights granted under\n", stdout);
    fputs("this License. No additional rights or licenses will be implied from the\n", stdout);
    fputs("distribution or licensing of Covered Software under this License.\n", stdout);
    fputs("Notwithstanding Section 2.1(b) above, no patent license is granted by a\n", stdout);
    fputs("Contributor:\n\n", stdout);

    fputs("(a) for any code that a Contributor has removed from Covered Software;\n", stdout);
    fputs("    or\n\n", stdout);

    fputs("(b) for infringements caused by: (i) Your and any other third party's\n", stdout);
    fputs("    modifications of Covered Software, or (ii) the combination of its\n", stdout);
    fputs("    Contributions with other software (except as part of its Contributor\n", stdout);
    fputs("    Version); or\n\n", stdout);

    fputs("(c) under Patent Claims infringed by Covered Software in the absence of\n", stdout);
    fputs("    its Contributions.\n\n", stdout);

    fputs("This License does not grant any rights in the trademarks, service marks,\n", stdout);
    fputs("or logos of any Contributor (except as may be necessary to comply with\n", stdout);
    fputs("the notice requirements in Section 3.4).\n\n", stdout);

    fputs("2.4. Subsequent Licenses\n\n", stdout);

    fputs("No Contributor makes additional grants as a result of Your choice to\n", stdout);
    fputs("distribute the Covered Software under a subsequent version of this\n", stdout);
    fputs("License (see Section 10.2) or under the terms of a Secondary License (if\n", stdout);
    fputs("permitted under the terms of Section 3.3).\n\n", stdout);

    fputs("2.5. Representation\n\n", stdout);

    fputs("Each Contributor represents that the Contributor believes its\n", stdout);
    fputs("Contributions are its original creation(s) or it has sufficient rights\n", stdout);
    fputs("to grant the rights to its Contributions conveyed by this License.\n\n", stdout);

    fputs("2.6. Fair Use\n\n", stdout);

    fputs("This License is not intended to limit any rights You have under\n", stdout);
    fputs("applicable copyright doctrines of fair use, fair dealing, or other\n", stdout);
    fputs("equivalents.\n\n", stdout);

    fputs("2.7. Conditions\n\n", stdout);

    fputs("Sections 3.1, 3.2, 3.3, and 3.4 are conditions of the licenses granted\n", stdout);
    fputs("in Section 2.1.\n\n", stdout);

    fputs("3. Responsibilities\n", stdout);
    fputs("-------------------\n\n", stdout);

    fputs("3.1. Distribution of Source Form\n\n", stdout);

    fputs("All distribution of Covered Software in Source Code Form, including any\n", stdout);
    fputs("Modifications that You create or to which You contribute, must be under\n", stdout);
    fputs("the terms of this License. You must inform recipients that the Source\n", stdout);
    fputs("Code Form of the Covered Software is governed by the terms of this\n", stdout);
    fputs("License, and how they can obtain a copy of this License. You may not\n", stdout);
    fputs("attempt to alter or restrict the recipients' rights in the Source Code\n", stdout);
    fputs("Form.\n\n", stdout);

    fputs("3.2. Distribution of Executable Form\n\n", stdout);

    fputs("If You distribute Covered Software in Executable Form then:\n\n", stdout);

    fputs("(a) such Covered Software must also be made available in Source Code\n", stdout);
    fputs("    Form, as described in Section 3.1, and You must inform recipients of\n", stdout);
    fputs("    the Executable Form how they can obtain a copy of such Source Code\n", stdout);
    fputs("    Form by reasonable means in a timely manner, at a charge no more\n", stdout);
    fputs("    than the cost of distribution to the recipient; and\n\n", stdout);

    fputs("(b) You may distribute such Executable Form under the terms of this\n", stdout);
    fputs("    License, or sublicense it under different terms, provided that the\n", stdout);
    fputs("    license for the Executable Form does not attempt to limit or alter\n", stdout);
    fputs("    the recipients' rights in the Source Code Form under this License.\n\n", stdout);

    fputs("3.3. Distribution of a Larger Work\n\n", stdout);

    fputs("You may create and distribute a Larger Work under terms of Your choice,\n", stdout);
    fputs("provided that You also comply with the requirements of this License for\n", stdout);
    fputs("the Covered Software. If the Larger Work is a combination of Covered\n", stdout);
    fputs("Software with a work governed by one or more Secondary Licenses, and the\n", stdout);
    fputs("Covered Software is not Incompatible With Secondary Licenses, this\n", stdout);
    fputs("License permits You to additionally distribute such Covered Software\n", stdout);
    fputs("under the terms of such Secondary License(s), so that the recipient of\n", stdout);
    fputs("the Larger Work may, at their option, further distribute the Covered\n", stdout);
    fputs("Software under the terms of either this License or such Secondary\n", stdout);
    fputs("License(s).\n\n", stdout);

    fputs("3.4. Notices\n\n", stdout);

    fputs("You may not remove or alter the substance of any license notices\n", stdout);
    fputs("(including copyright notices, patent notices, disclaimers of warranty,\n", stdout);
    fputs("or limitations of liability) contained within the Source Code Form of\n", stdout);
    fputs("the Covered Software, except that You may alter any license notices to\n", stdout);
    fputs("the extent required to remedy known factual inaccuracies.\n\n", stdout);

    fputs("3.5. Application of Additional Terms\n\n", stdout);

    fputs("You may choose to offer, and to charge a fee for, warranty, support,\n", stdout);
    fputs("indemnity or liability obligations to one or more recipients of Covered\n", stdout);
    fputs("Software. However, You may do so only on Your own behalf, and not on\n", stdout);
    fputs("behalf of any Contributor. You must make it absolutely clear that any\n", stdout);
    fputs("such warranty, support, indemnity, or liability obligation is offered by\n", stdout);
    fputs("You alone, and You hereby agree to indemnify every Contributor for any\n", stdout);
    fputs("liability incurred by such Contributor as a result of warranty, support,\n", stdout);
    fputs("indemnity or liability terms You offer. You may include additional\n", stdout);
    fputs("disclaimers of warranty and limitations of liability specific to any\n", stdout);
    fputs("jurisdiction.\n\n", stdout);

    fputs("4. Inability to Comply Due to Statute or Regulation\n", stdout);
    fputs("---------------------------------------------------\n\n", stdout);

    fputs("If it is impossible for You to comply with any of the terms of this\n", stdout);
    fputs("License with respect to some or all of the Covered Software due to\n", stdout);
    fputs("statute, judicial order, or regulation then You must: (a) comply with\n", stdout);
    fputs("the terms of this License to the maximum extent possible; and (b)\n", stdout);
    fputs("describe the limitations and the code they affect. Such description must\n", stdout);
    fputs("be placed in a text file included with all distributions of the Covered\n", stdout);
    fputs("Software under this License. Except to the extent prohibited by statute\n", stdout);
    fputs("or regulation, such description must be sufficiently detailed for a\n", stdout);
    fputs("recipient of ordinary skill to be able to understand it.\n\n", stdout);

    fputs("5. Termination\n", stdout);
    fputs("--------------\n\n", stdout);

    fputs("5.1. The rights granted under this License will terminate automatically\n", stdout);
    fputs("if You fail to comply with any of its terms. However, if You become\n", stdout);
    fputs("compliant, then the rights granted under this License from a particular\n", stdout);
    fputs("Contributor are reinstated (a) provisionally, unless and until such\n", stdout);
    fputs("Contributor explicitly and finally terminates Your grants, and (b) on an\n", stdout);
    fputs("ongoing basis, if such Contributor fails to notify You of the\n", stdout);
    fputs("non-compliance by some reasonable means prior to 60 days after You have\n", stdout);
    fputs("come back into compliance. Moreover, Your grants from a particular\n", stdout);
    fputs("Contributor are reinstated on an ongoing basis if such Contributor\n", stdout);
    fputs("notifies You of the non-compliance by some reasonable means, this is the\n", stdout);
    fputs("first time You have received notice of non-compliance with this License\n", stdout);
    fputs("from such Contributor, and You become compliant prior to 30 days after\n", stdout);
    fputs("Your receipt of the notice.\n\n", stdout);

    fputs("5.2. If You initiate litigation against any entity by asserting a patent\n", stdout);
    fputs("infringement claim (excluding declaratory judgment actions,\n", stdout);
    fputs("counter-claims, and cross-claims) alleging that a Contributor Version\n", stdout);
    fputs("directly or indirectly infringes any patent, then the rights granted to\n", stdout);
    fputs("You by any and all Contributors for the Covered Software under Section\n", stdout);
    fputs("2.1 of this License shall terminate.\n\n", stdout);

    fputs("5.3. In the event of termination under Sections 5.1 or 5.2 above, all\n", stdout);
    fputs("end user license agreements (excluding distributors and resellers) which\n", stdout);
    fputs("have been validly granted by You or Your distributors under this License\n", stdout);
    fputs("prior to termination shall survive termination.\n\n", stdout);

    fputs("************************************************************************\n", stdout);
    fputs("*                                                                      *\n", stdout);
    fputs("*  6. Disclaimer of Warranty                                           *\n", stdout);
    fputs("*  -------------------------                                           *\n", stdout);
    fputs("*                                                                      *\n", stdout);
    fputs("*  Covered Software is provided under this License on an \"as is\"       *\n", stdout);
    fputs("*  basis, without warranty of any kind, either expressed, implied, or  *\n", stdout);
    fputs("*  statutory, including, without limitation, warranties that the       *\n", stdout);
    fputs("*  Covered Software is free of defects, merchantable, fit for a        *\n", stdout);
    fputs("*  particular purpose or non-infringing. The entire risk as to the     *\n", stdout);
    fputs("*  quality and performance of the Covered Software is with You.        *\n", stdout);
    fputs("*  Should any Covered Software prove defective in any respect, You     *\n", stdout);
    fputs("*  (not any Contributor) assume the cost of any necessary servicing,   *\n", stdout);
    fputs("*  repair, or correction. This disclaimer of warranty constitutes an   *\n", stdout);
    fputs("*  essential part of this License. No use of any Covered Software is   *\n", stdout);
    fputs("*  authorized under this License except under this disclaimer.         *\n", stdout);
    fputs("*                                                                      *\n", stdout);
    fputs("************************************************************************\n\n", stdout);

    fputs("************************************************************************\n", stdout);
    fputs("*                                                                      *\n", stdout);
    fputs("*  7. Limitation of Liability                                          *\n", stdout);
    fputs("*  --------------------------                                          *\n", stdout);
    fputs("*                                                                      *\n", stdout);
    fputs("*  Under no circumstances and under no legal theory, whether tort      *\n", stdout);
    fputs("*  (including negligence), contract, or otherwise, shall any           *\n", stdout);
    fputs("*  Contributor, or anyone who distributes Covered Software as          *\n", stdout);
    fputs("*  permitted above, be liable to You for any direct, indirect,         *\n", stdout);
    fputs("*  special, incidental, or consequential damages of any character      *\n", stdout);
    fputs("*  including, without limitation, damages for lost profits, loss of    *\n", stdout);
    fputs("*  goodwill, work stoppage, computer failure or malfunction, or any    *\n", stdout);
    fputs("*  and all other commercial damages or losses, even if such party      *\n", stdout);
    fputs("*  shall have been informed of the possibility of such damages. This   *\n", stdout);
    fputs("*  limitation of liability shall not apply to liability for death or   *\n", stdout);
    fputs("*  personal injury resulting from such party's negligence to the       *\n", stdout);
    fputs("*  extent applicable law prohibits such limitation. Some               *\n", stdout);
    fputs("*  jurisdictions do not allow the exclusion or limitation of           *\n", stdout);
    fputs("*  incidental or consequential damages, so this exclusion and          *\n", stdout);
    fputs("*  limitation may not apply to You.                                    *\n", stdout);
    fputs("*                                                                      *\n", stdout);
    fputs("************************************************************************\n\n", stdout);

    fputs("8. Litigation\n", stdout);
    fputs("-------------\n\n", stdout);

    fputs("Any litigation relating to this License may be brought only in the\n", stdout);
    fputs("courts of a jurisdiction where the defendant maintains its principal\n", stdout);
    fputs("place of business and such litigation shall be governed by laws of that\n", stdout);
    fputs("jurisdiction, without reference to its conflict-of-law provisions.\n", stdout);
    fputs("Nothing in this Section shall prevent a party's ability to bring\n", stdout);
    fputs("cross-claims or counter-claims.\n\n", stdout);

    fputs("9. Miscellaneous\n", stdout);
    fputs("----------------\n\n", stdout);

    fputs("This License represents the complete agreement concerning the subject\n", stdout);
    fputs("matter hereof. If any provision of this License is held to be\n", stdout);
    fputs("unenforceable, such provision shall be reformed only to the extent\n", stdout);
    fputs("necessary to make it enforceable. Any law or regulation which provides\n", stdout);
    fputs("that the language of a contract shall be construed against the drafter\n", stdout);
    fputs("shall not be used to construe this License against a Contributor.\n\n", stdout);

    fputs("10. Versions of the License\n", stdout);
    fputs("---------------------------\n\n", stdout);

    fputs("10.1. New Versions\n\n", stdout);

    fputs("Mozilla Foundation is the license steward. Except as provided in Section\n", stdout);
    fputs("10.3, no one other than the license steward has the right to modify or\n", stdout);
    fputs("publish new versions of this License. Each version will be given a\n", stdout);
    fputs("distinguishing version number.\n\n", stdout);

    fputs("10.2. Effect of New Versions\n\n", stdout);

    fputs("You may distribute the Covered Software under the terms of the version\n", stdout);
    fputs("of the License under which You originally received the Covered Software,\n", stdout);
    fputs("or under the terms of any subsequent version published by the license\n", stdout);
    fputs("steward.\n\n", stdout);

    fputs("10.3. Modified Versions\n\n", stdout);

    fputs("If you create software not governed by this License, and you want to\n", stdout);
    fputs("create a new license for such software, you may create and use a\n", stdout);
    fputs("modified version of this License if you rename the license and remove\n", stdout);
    fputs("any references to the name of the license steward (except to note that\n", stdout);
    fputs("such modified license differs from this License).\n\n", stdout);

    fputs("10.4. Distributing Source Code Form that is Incompatible With Secondary\n", stdout);
    fputs("Licenses\n\n", stdout);

    fputs("If You choose to distribute Source Code Form that is Incompatible With\n", stdout);
    fputs("Secondary Licenses under the terms of this version of the License, the\n", stdout);
    fputs("notice described in Exhibit B of this License must be attached.\n\n", stdout);

    fputs("Exhibit A - Source Code Form License Notice\n", stdout);
    fputs("-------------------------------------------\n\n", stdout);

    fputs("  This Source Code Form is subject to the terms of the Mozilla Public\n", stdout);
    fputs("  License, v. 2.0. If a copy of the MPL was not distributed with this\n", stdout);
    fputs("  file, You can obtain one at http://mozilla.org/MPL/2.0/.\n\n", stdout);

    fputs("If it is not possible or desirable to put the notice in a particular\n", stdout);
    fputs("file, then You may include the notice in a location (such as a LICENSE\n", stdout);
    fputs("file in a relevant directory) where a recipient would be likely to look\n", stdout);
    fputs("for such a notice.\n\n", stdout);

    fputs("You may add additional accurate notices of copyright ownership.\n\n", stdout);

    fputs("Exhibit B - \"Incompatible With Secondary Licenses\" Notice\n", stdout);
    fputs("---------------------------------------------------------\n\n", stdout);

    fputs("  This Source Code Form is \"Incompatible With Secondary Licenses\", as\n", stdout);
    fputs("  defined by the Mozilla Public License, v. 2.0.\n\n", stdout);
    print_Open_Source_Licenses();
}

static void print_Win_Getopt_Licenses(void)
{
    fputs("===========================================================================\n", stdout);
    fputs("wingetopt - getopt library for Windows compilers\n\n", stdout);

    fputs("This library was created to allow compilation linux-based software on Windows.\n", stdout);
    fputs("http://en.wikipedia.org/wiki/Getopt  The sources were taken from MinGW-runtime\n", stdout);
    fputs("project.\n\n", stdout);

    fputs("AUTHORS: Todd C. Miller Todd.Miller@courtesan.com; The NetBSD Foundation, Inc.\n\n", stdout);

    fputs("LICENSE\n\n", stdout);

    fputs("Copyright (c) 2002 Todd C. Miller <Todd.Miller@courtesan.com>\n\n", stdout);

    fputs("Permission to use, copy, modify, and distribute this software for any purpose\n", stdout);
    fputs("with or without fee is hereby granted, provided that the above copyright notice\n", stdout);
    fputs("and this permission notice appear in all copies.\n\n", stdout);

    fputs("THE SOFTWARE IS PROVIDED \"AS IS\" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH\n", stdout);
    fputs("REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND\n", stdout);
    fputs("FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,\n", stdout);
    fputs("INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM\n", stdout);
    fputs("LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR\n", stdout);
    fputs("OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR\n", stdout);
    fputs("PERFORMANCE OF THIS SOFTWARE.\n\n", stdout);

    fputs("Sponsored in part by the Defense Advanced Research Projects Agency (DARPA) and\n", stdout);
    fputs("Air Force Research Laboratory, Air Force Materiel Command, USAF, under\n", stdout);
    fputs("agreement number F39502-99-1-0512.\n\n", stdout);

    fputs("                 -------------------------------------\n\n", stdout);

    fputs("Copyright (c) 2000 The NetBSD Foundation, Inc.\n", stdout);
    fputs("All rights reserved.\n\n", stdout);

    fputs("This code is derived from software contributed to The NetBSD Foundation\n", stdout);
    fputs("by Dieter Baron and Thomas Klausner.\n\n", stdout);

    fputs("Redistribution and use in source and binary forms, with or without\n", stdout);
    fputs("modification, are permitted provided that the following conditions are met:\n", stdout);
    fputs("1. Redistributions of source code must retain the above copyright notice, this\n", stdout);
    fputs("   list of conditions and the following disclaimer.\n", stdout);
    fputs("2. Redistributions in binary form must reproduce the above copyright notice,\n", stdout);
    fputs("   this list of conditions and the following disclaimer in the documentation\n", stdout);
    fputs("   and/or other materials provided with the distribution.\n\n", stdout);

    fputs("THIS SOFTWARE IS PROVIDED BY THE NETBSD FOUNDATION, INC. AND CONTRIBUTORS \"AS\n", stdout);
    fputs("IS\" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE\n", stdout);
    fputs("IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE\n", stdout);
    fputs("DISCLAIMED.  IN NO EVENT SHALL THE FOUNDATION OR CONTRIBUTORS BE LIABLE FOR ANY\n", stdout);
    fputs("DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES\n", stdout);
    fputs("(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;\n", stdout);
    fputs("LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON\n", stdout);
    fputs("ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT\n", stdout);
    fputs("(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS\n", stdout);
    fputs("SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.\n\n", stdout);
}

#if defined(__FreeBSD__) || defined(__DragonFly__) || defined(__NetBSD__) || defined(__OpenBSD__)
static void print_Berkeley_License(void)
{
    fputs("Berkeley Copyright to BSD source (libc, others)\n", stdout);
    fputs("Copyright (c) 1990 The Regents of the University of California.\n", stdout);
    fputs("All rights reserved.\n", stdout);
    fputs("\n", stdout);
    fputs("Redistribution and use in source and binary forms, with or without\n", stdout);
    fputs("modification, are permitted provided that the following conditions\n", stdout);
    fputs("are met:\n", stdout);
    fputs("1. Redistributions of source code must retain the above copyright\n", stdout);
    fputs("   notice, this list of conditions and the following disclaimer.\n", stdout);
    fputs("2. Redistributions in binary form must reproduce the above copyright\n", stdout);
    fputs("   notice, this list of conditions and the following disclaimer in the\n", stdout);
    fputs("   documentation and/or other materials provided with the distribution.\n", stdout);
    fputs("3. Neither the name of the University nor the names of its contributors\n", stdout);
    fputs("   may be used to endorse or promote products derived from this software\n", stdout);
    fputs("   without specific prior written permission.\n", stdout);
    fputs("\n", stdout);
    fputs("THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND\n", stdout);
    fputs("ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE\n", stdout);
    fputs("IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE\n", stdout);
    fputs("ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE\n", stdout);
    fputs("FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL\n", stdout);
    fputs("DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS\n", stdout);
    fputs("OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)\n", stdout);
    fputs("HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT\n", stdout);
    fputs("LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY\n", stdout);
    fputs("OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF\n", stdout);
    fputs("SUCH DAMAGE.\n", stdout);
    fputs("\n\n", stdout);
}
#endif // all bsds

#if defined(__FreeBSD__)
static void print_FreeBSD_License(void)
{
    fputs("FreeBSD (libc)\n\n", stdout);
    fputs("Copyright 1992 - 2025 The FreeBSD Project.\n\n", stdout);
    fputs("Redistribution and use in source and binary forms, with or without\n", stdout);
    fputs("modification, are permitted provided that the following conditions are met :\n", stdout);
    fputs("\n", stdout);
    fputs("1. Redistributions of source code must retain the above copyright notice,\n", stdout);
    fputs("this list of conditions and the following disclaimer.\n", stdout);
    fputs("2. Redistributions in binary form must reproduce the above copyright notice,\n", stdout);
    fputs("this list of conditions and the following disclaimer in the documentation\n", stdout);
    fputs("and/or other materials provided with the distribution.\n", stdout);
    fputs("\n", stdout);
    fputs("THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND ANY\n", stdout);
    fputs("EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED\n", stdout);
    fputs("WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.\n", stdout);
    fputs("IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,\n", stdout);
    fputs("INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES(INCLUDING, BUT NOT\n", stdout);
    fputs("LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,\n", stdout);
    fputs("OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF\n", stdout);
    fputs("LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT(INCLUDING NEGLIGENCE\n", stdout);
    fputs("OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED\n", stdout);
    fputs("OF THE POSSIBILITY OF SUCH DAMAGE.\n", stdout);
    fputs("\n", stdout);
    fputs("The views and conclusions contained in the software and documentation are those of\n", stdout);
    fputs("the authors and should not be interpreted as representing official policies, either\n", stdout);
    fputs("expressed or implied, of the FreeBSD Project.\n", stdout);
    fputs("\n\n", stdout);
    print_Berkeley_License();
}
#endif //__FreeBSD__

#if defined(__DragonFly__)
static void print_DragonFlyBSD_License(void)
{
    fputs("DragonflyBSD (libc)\n\n", stdout);
    fputs("Copyright (c) 2003-2024 The DragonFly Project.  All rights reserved.\n\n", stdout);
    fputs("Redistribution and use in source and binary forms, with or without\n", stdout);
    fputs("modification, are permitted provided that the following conditions are met:\n", stdout);
    fputs("\n", stdout);
    fputs("1. Redistributions of source code must retain the above copyright\n", stdout);
    fputs("   notice, this list of conditions and the following disclaimer.\n", stdout);
    fputs("2. Redistributions in binary form must reproduce the above copyright\n", stdout);
    fputs("   notice, this list of conditions and the following disclaimer in\n", stdout);
    fputs("   the documentation and/or other materials provided with the\n", stdout);
    fputs("   distribution.\n", stdout);
    fputs("3. Neither the name of The DragonFly Project nor the names of its\n", stdout);
    fputs("   contributors may be used to endorse or promote products derived\n", stdout);
    fputs("   from this software without specific, prior written permission.\n", stdout);
    fputs("\n", stdout);
    fputs("THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS\n", stdout);
    fputs("``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT\n", stdout);
    fputs("LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS\n", stdout);
    fputs("FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE\n", stdout);
    fputs("COPYRIGHT HOLDERS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,\n", stdout);
    fputs("INCIDENTAL, SPECIAL, EXEMPLARY OR CONSEQUENTIAL DAMAGES (INCLUDING,\n", stdout);
    fputs("BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;\n", stdout);
    fputs("LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED\n", stdout);
    fputs("AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,\n", stdout);
    fputs("OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT\n", stdout);
    fputs("OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF\n", stdout);
    fputs("SUCH DAMAGE.\n", stdout);
    fputs("\n\n", stdout);
    print_Berkeley_License();
}

#endif //__DragonFly__

#if defined(__NetBSD__)
static void print_NetBSD_License(void)
{
    fputs("NetBSD (libc)\n\n", stdout);
    fputs("Copyright (c) 2008 The NetBSD Foundation, Inc.\n", stdout);
    fputs("All rights reserved.\n", stdout);
    fputs("\n", stdout);
    fputs("This code is derived from software contributed to The NetBSD Foundation\n", stdout);
    fputs("by \n", stdout);
    fputs("\n", stdout);
    fputs("Redistribution and use in source and binary forms, with or without\n", stdout);
    fputs("modification, are permitted provided that the following conditions\n", stdout);
    fputs("are met:\n", stdout);
    fputs("1. Redistributions of source code must retain the above copyright\n", stdout);
    fputs("   notice, this list of conditions and the following disclaimer.\n", stdout);
    fputs("2. Redistributions in binary form must reproduce the above copyright\n", stdout);
    fputs("   notice, this list of conditions and the following disclaimer in the\n", stdout);
    fputs("   documentation and/or other materials provided with the distribution.\n", stdout);
    fputs("\n", stdout);
    fputs("THIS SOFTWARE IS PROVIDED BY THE NETBSD FOUNDATION, INC. AND CONTRIBUTORS\n", stdout);
    fputs("``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED\n", stdout);
    fputs("TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR\n", stdout);
    fputs("PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE FOUNDATION OR CONTRIBUTORS\n", stdout);
    fputs("BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR\n", stdout);
    fputs("CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF\n", stdout);
    fputs("SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS\n", stdout);
    fputs("INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN\n", stdout);
    fputs("CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)\n", stdout);
    fputs("ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE\n", stdout);
    fputs("POSSIBILITY OF SUCH DAMAGE.\n", stdout);
    fputs("\n\n", stdout);
    print_Berkeley_License();
}
#endif //__NetBSD__

#if defined(__OpenBSD__)
static void print_OpenBSD_License(void)
{
    fputs("OpenBSD (libc)\n\n", stdout);
    fputs("Copyright (c) 2003 The OpenBSD Foundation\n", stdout);
    fputs("\n", stdout);
    fputs("Permission to use, copy, modify, and distribute this software for any\n", stdout);
    fputs("purpose with or without fee is hereby granted, provided that the above\n", stdout);
    fputs("copyright notice and this permission notice appear in all copies.\n", stdout);
    fputs("\n", stdout);
    fputs("THE SOFTWARE IS PROVIDED \"AS IS\" AND THE AUTHOR DISCLAIMS ALL WARRANTIES\n", stdout);
    fputs("WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF\n", stdout);
    fputs("MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR\n", stdout);
    fputs("ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES\n", stdout);
    fputs("WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN\n", stdout);
    fputs("ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF\n", stdout);
    fputs("OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.\n", stdout);
    fputs("\n\n", stdout);
    print_Berkeley_License();
}
#endif //__OpenBSD__

#if defined(__sun)
#    if defined(__illumos__) || defined(THIS_IS_ILLUMOS)
static void print_CDDL_License(void)
{
    fputs("Illumos (libc)\n\n", stdout);
    fputs("COMMON DEVELOPMENT AND DISTRIBUTION LICENSE Version 1.0\n", stdout);
    fputs("\n", stdout);
    fputs("1. Definitions.\n", stdout);
    fputs("\n", stdout);
    fputs("    1.1. \"Contributor\" means each individual or entity that creates\n", stdout);
    fputs("         or contributes to the creation of Modifications.\n", stdout);
    fputs("\n", stdout);
    fputs("    1.2. \"Contributor Version\" means the combination of the Original\n", stdout);
    fputs("         Software, prior Modifications used by a Contributor (if any),\n", stdout);
    fputs("         and the Modifications made by that particular Contributor.\n", stdout);
    fputs("\n", stdout);
    fputs("    1.3. \"Covered Software\" means (a) the Original Software, or (b)\n", stdout);
    fputs("         Modifications, or (c) the combination of files containing\n", stdout);
    fputs("         Original Software with files containing Modifications, in\n", stdout);
    fputs("         each case including portions thereof.\n", stdout);
    fputs("\n", stdout);
    fputs("    1.4. \"Executable\" means the Covered Software in any form other\n", stdout);
    fputs("         than Source Code.\n", stdout);
    fputs("\n", stdout);
    fputs("    1.5. \"Initial Developer\" means the individual or entity that first\n", stdout);
    fputs("         makes Original Software available under this License.\n", stdout);
    fputs("\n", stdout);
    fputs("    1.6. \"Larger Work\" means a work which combines Covered Software or\n", stdout);
    fputs("         portions thereof with code not governed by the terms of this\n", stdout);
    fputs("         License.\n", stdout);
    fputs("\n", stdout);
    fputs("    1.7. \"License\" means this document.\n", stdout);
    fputs("\n", stdout);
    fputs("    1.8. \"Licensable\" means having the right to grant, to the maximum\n", stdout);
    fputs("         extent possible, whether at the time of the initial grant or\n", stdout);
    fputs("         subsequently acquired, any and all of the rights conveyed\n", stdout);
    fputs("         herein.\n", stdout);
    fputs("\n", stdout);
    fputs("    1.9. \"Modifications\" means the Source Code and Executable form of\n", stdout);
    fputs("         any of the following:\n", stdout);
    fputs("\n", stdout);
    fputs("        A. Any file that results from an addition to, deletion from or\n", stdout);
    fputs("           modification of the contents of a file containing Original\n", stdout);
    fputs("           Software or previous Modifications;\n", stdout);
    fputs("\n", stdout);
    fputs("        B. Any new file that contains any part of the Original\n", stdout);
    fputs("           Software or previous Modifications; or\n", stdout);
    fputs("\n", stdout);
    fputs("        C. Any new file that is contributed or otherwise made\n", stdout);
    fputs("           available under the terms of this License.\n", stdout);
    fputs("\n", stdout);
    fputs("    1.10. \"Original Software\" means the Source Code and Executable\n", stdout);
    fputs("          form of computer software code that is originally released\n", stdout);
    fputs("          under this License.\n", stdout);
    fputs("\n", stdout);
    fputs("    1.11. \"Patent Claims\" means any patent claim(s), now owned or\n", stdout);
    fputs("          hereafter acquired, including without limitation, method,\n", stdout);
    fputs("          process, and apparatus claims, in any patent Licensable by\n", stdout);
    fputs("          grantor.\n", stdout);
    fputs("\n", stdout);
    fputs("    1.12. \"Source Code\" means (a) the common form of computer software\n", stdout);
    fputs("          code in which modifications are made and (b) associated\n", stdout);
    fputs("          documentation included in or with such code.\n", stdout);
    fputs("\n", stdout);
    fputs("    1.13. \"You\" (or \"Your\") means an individual or a legal entity\n", stdout);
    fputs("          exercising rights under, and complying with all of the terms\n", stdout);
    fputs("          of, this License.  For legal entities, \"You\" includes any\n", stdout);
    fputs("          entity which controls, is controlled by, or is under common\n", stdout);
    fputs("          control with You.  For purposes of this definition,\n", stdout);
    fputs("          \"control\" means (a) the power, direct or indirect, to cause\n", stdout);
    fputs("          the direction or management of such entity, whether by\n", stdout);
    fputs("          contract or otherwise, or (b) ownership of more than fifty\n", stdout);
    fputs("          percent (50%%) of the outstanding shares or beneficial\n", stdout);
    fputs("          ownership of such entity.\n", stdout);
    fputs("\n", stdout);
    fputs("2. License Grants.\n", stdout);
    fputs("\n", stdout);
    fputs("    2.1. The Initial Developer Grant.\n", stdout);
    fputs("\n", stdout);
    fputs("    Conditioned upon Your compliance with Section 3.1 below and\n", stdout);
    fputs("    subject to third party intellectual property claims, the Initial\n", stdout);
    fputs("    Developer hereby grants You a world-wide, royalty-free,\n", stdout);
    fputs("    non-exclusive license:\n", stdout);
    fputs("\n", stdout);
    fputs("        (a) under intellectual property rights (other than patent or\n", stdout);
    fputs("            trademark) Licensable by Initial Developer, to use,\n", stdout);
    fputs("            reproduce, modify, display, perform, sublicense and\n", stdout);
    fputs("            distribute the Original Software (or portions thereof),\n", stdout);
    fputs("            with or without Modifications, and/or as part of a Larger\n", stdout);
    fputs("            Work; and\n", stdout);
    fputs("\n", stdout);
    fputs("        (b) under Patent Claims infringed by the making, using or\n", stdout);
    fputs("            selling of Original Software, to make, have made, use,\n", stdout);
    fputs("            practice, sell, and offer for sale, and/or otherwise\n", stdout);
    fputs("            dispose of the Original Software (or portions thereof).\n", stdout);
    fputs("\n", stdout);
    fputs("        (c) The licenses granted in Sections 2.1(a) and (b) are\n", stdout);
    fputs("            effective on the date Initial Developer first distributes\n", stdout);
    fputs("            or otherwise makes the Original Software available to a\n", stdout);
    fputs("            third party under the terms of this License.\n", stdout);
    fputs("\n", stdout);
    fputs("        (d) Notwithstanding Section 2.1(b) above, no patent license is\n", stdout);
    fputs("            granted: (1) for code that You delete from the Original\n", stdout);
    fputs("            Software, or (2) for infringements caused by: (i) the\n", stdout);
    fputs("            modification of the Original Software, or (ii) the\n", stdout);
    fputs("            combination of the Original Software with other software\n", stdout);
    fputs("            or devices.\n", stdout);
    fputs("\n", stdout);
    fputs("    2.2. Contributor Grant.\n", stdout);
    fputs("\n", stdout);
    fputs("    Conditioned upon Your compliance with Section 3.1 below and\n", stdout);
    fputs("    subject to third party intellectual property claims, each\n", stdout);
    fputs("    Contributor hereby grants You a world-wide, royalty-free,\n", stdout);
    fputs("    non-exclusive license:\n", stdout);
    fputs("\n", stdout);
    fputs("        (a) under intellectual property rights (other than patent or\n", stdout);
    fputs("            trademark) Licensable by Contributor to use, reproduce,\n", stdout);
    fputs("            modify, display, perform, sublicense and distribute the\n", stdout);
    fputs("            Modifications created by such Contributor (or portions\n", stdout);
    fputs("            thereof), either on an unmodified basis, with other\n", stdout);
    fputs("            Modifications, as Covered Software and/or as part of a\n", stdout);
    fputs("            Larger Work; and\n", stdout);
    fputs("\n", stdout);
    fputs("        (b) under Patent Claims infringed by the making, using, or\n", stdout);
    fputs("            selling of Modifications made by that Contributor either\n", stdout);
    fputs("            alone and/or in combination with its Contributor Version\n", stdout);
    fputs("            (or portions of such combination), to make, use, sell,\n", stdout);
    fputs("            offer for sale, have made, and/or otherwise dispose of:\n", stdout);
    fputs("            (1) Modifications made by that Contributor (or portions\n", stdout);
    fputs("            thereof); and (2) the combination of Modifications made by\n", stdout);
    fputs("            that Contributor with its Contributor Version (or portions\n", stdout);
    fputs("            of such combination).\n", stdout);
    fputs("\n", stdout);
    fputs("        (c) The licenses granted in Sections 2.2(a) and 2.2(b) are\n", stdout);
    fputs("            effective on the date Contributor first distributes or\n", stdout);
    fputs("            otherwise makes the Modifications available to a third\n", stdout);
    fputs("            party.\n", stdout);
    fputs("\n", stdout);
    fputs("        (d) Notwithstanding Section 2.2(b) above, no patent license is\n", stdout);
    fputs("            granted: (1) for any code that Contributor has deleted\n", stdout);
    fputs("            from the Contributor Version; (2) for infringements caused\n", stdout);
    fputs("            by: (i) third party modifications of Contributor Version,\n", stdout);
    fputs("            or (ii) the combination of Modifications made by that\n", stdout);
    fputs("            Contributor with other software (except as part of the\n", stdout);
    fputs("            Contributor Version) or other devices; or (3) under Patent\n", stdout);
    fputs("            Claims infringed by Covered Software in the absence of\n", stdout);
    fputs("            Modifications made by that Contributor.\n", stdout);
    fputs("\n", stdout);
    fputs("3. Distribution Obligations.\n", stdout);
    fputs("\n", stdout);
    fputs("    3.1. Availability of Source Code.\n", stdout);
    fputs("\n", stdout);
    fputs("    Any Covered Software that You distribute or otherwise make\n", stdout);
    fputs("    available in Executable form must also be made available in Source\n", stdout);
    fputs("    Code form and that Source Code form must be distributed only under\n", stdout);
    fputs("    the terms of this License.  You must include a copy of this\n", stdout);
    fputs("    License with every copy of the Source Code form of the Covered\n", stdout);
    fputs("    Software You distribute or otherwise make available.  You must\n", stdout);
    fputs("    inform recipients of any such Covered Software in Executable form\n", stdout);
    fputs("    as to how they can obtain such Covered Software in Source Code\n", stdout);
    fputs("    form in a reasonable manner on or through a medium customarily\n", stdout);
    fputs("    used for software exchange.\n", stdout);
    fputs("\n", stdout);
    fputs("    3.2. Modifications.\n", stdout);
    fputs("\n", stdout);
    fputs("    The Modifications that You create or to which You contribute are\n", stdout);
    fputs("    governed by the terms of this License.  You represent that You\n", stdout);
    fputs("    believe Your Modifications are Your original creation(s) and/or\n", stdout);
    fputs("    You have sufficient rights to grant the rights conveyed by this\n", stdout);
    fputs("    License.\n", stdout);
    fputs("\n", stdout);
    fputs("    3.3. Required Notices.\n", stdout);
    fputs("\n", stdout);
    fputs("    You must include a notice in each of Your Modifications that\n", stdout);
    fputs("    identifies You as the Contributor of the Modification.  You may\n", stdout);
    fputs("    not remove or alter any copyright, patent or trademark notices\n", stdout);
    fputs("    contained within the Covered Software, or any notices of licensing\n", stdout);
    fputs("    or any descriptive text giving attribution to any Contributor or\n", stdout);
    fputs("    the Initial Developer.\n", stdout);
    fputs("\n", stdout);
    fputs("    3.4. Application of Additional Terms.\n", stdout);
    fputs("\n", stdout);
    fputs("    You may not offer or impose any terms on any Covered Software in\n", stdout);
    fputs("    Source Code form that alters or restricts the applicable version\n", stdout);
    fputs("    of this License or the recipients' rights hereunder.  You may\n", stdout);
    fputs("    choose to offer, and to charge a fee for, warranty, support,\n", stdout);
    fputs("    indemnity or liability obligations to one or more recipients of\n", stdout);
    fputs("    Covered Software.  However, you may do so only on Your own behalf,\n", stdout);
    fputs("    and not on behalf of the Initial Developer or any Contributor.\n", stdout);
    fputs("    You must make it absolutely clear that any such warranty, support,\n", stdout);
    fputs("    indemnity or liability obligation is offered by You alone, and You\n", stdout);
    fputs("    hereby agree to indemnify the Initial Developer and every\n", stdout);
    fputs("    Contributor for any liability incurred by the Initial Developer or\n", stdout);
    fputs("    such Contributor as a result of warranty, support, indemnity or\n", stdout);
    fputs("    liability terms You offer.\n", stdout);
    fputs("\n", stdout);
    fputs("    3.5. Distribution of Executable Versions.\n", stdout);
    fputs("\n", stdout);
    fputs("    You may distribute the Executable form of the Covered Software\n", stdout);
    fputs("    under the terms of this License or under the terms of a license of\n", stdout);
    fputs("    Your choice, which may contain terms different from this License,\n", stdout);
    fputs("    provided that You are in compliance with the terms of this License\n", stdout);
    fputs("    and that the license for the Executable form does not attempt to\n", stdout);
    fputs("    limit or alter the recipient's rights in the Source Code form from\n", stdout);
    fputs("    the rights set forth in this License.  If You distribute the\n", stdout);
    fputs("    Covered Software in Executable form under a different license, You\n", stdout);
    fputs("    must make it absolutely clear that any terms which differ from\n", stdout);
    fputs("    this License are offered by You alone, not by the Initial\n", stdout);
    fputs("    Developer or Contributor.  You hereby agree to indemnify the\n", stdout);
    fputs("    Initial Developer and every Contributor for any liability incurred\n", stdout);
    fputs("    by the Initial Developer or such Contributor as a result of any\n", stdout);
    fputs("    such terms You offer.\n", stdout);
    fputs("\n", stdout);
    fputs("    3.6. Larger Works.\n", stdout);
    fputs("\n", stdout);
    fputs("    You may create a Larger Work by combining Covered Software with\n", stdout);
    fputs("    other code not governed by the terms of this License and\n", stdout);
    fputs("    distribute the Larger Work as a single product.  In such a case,\n", stdout);
    fputs("    You must make sure the requirements of this License are fulfilled\n", stdout);
    fputs("    for the Covered Software.\n", stdout);
    fputs("\n", stdout);
    fputs("4. Versions of the License.\n", stdout);
    fputs("\n", stdout);
    fputs("    4.1. New Versions.\n", stdout);
    fputs("\n", stdout);
    fputs("    Sun Microsystems, Inc. is the initial license steward and may\n", stdout);
    fputs("    publish revised and/or new versions of this License from time to\n", stdout);
    fputs("    time.  Each version will be given a distinguishing version number.\n", stdout);
    fputs("    Except as provided in Section 4.3, no one other than the license\n", stdout);
    fputs("    steward has the right to modify this License.\n", stdout);
    fputs("\n", stdout);
    fputs("    4.2. Effect of New Versions.\n", stdout);
    fputs("\n", stdout);
    fputs("    You may always continue to use, distribute or otherwise make the\n", stdout);
    fputs("    Covered Software available under the terms of the version of the\n", stdout);
    fputs("    License under which You originally received the Covered Software.\n", stdout);
    fputs("    If the Initial Developer includes a notice in the Original\n", stdout);
    fputs("    Software prohibiting it from being distributed or otherwise made\n", stdout);
    fputs("    available under any subsequent version of the License, You must\n", stdout);
    fputs("    distribute and make the Covered Software available under the terms\n", stdout);
    fputs("    of the version of the License under which You originally received\n", stdout);
    fputs("    the Covered Software.  Otherwise, You may also choose to use,\n", stdout);
    fputs("    distribute or otherwise make the Covered Software available under\n", stdout);
    fputs("    the terms of any subsequent version of the License published by\n", stdout);
    fputs("    the license steward.\n", stdout);
    fputs("\n", stdout);
    fputs("    4.3. Modified Versions.\n", stdout);
    fputs("\n", stdout);
    fputs("    When You are an Initial Developer and You want to create a new\n", stdout);
    fputs("    license for Your Original Software, You may create and use a\n", stdout);
    fputs("    modified version of this License if You: (a) rename the license\n", stdout);
    fputs("    and remove any references to the name of the license steward\n", stdout);
    fputs("    (except to note that the license differs from this License); and\n", stdout);
    fputs("    (b) otherwise make it clear that the license contains terms which\n", stdout);
    fputs("    differ from this License.\n", stdout);
    fputs("\n", stdout);
    fputs("5. DISCLAIMER OF WARRANTY.\n", stdout);
    fputs("\n", stdout);
    fputs("    COVERED SOFTWARE IS PROVIDED UNDER THIS LICENSE ON AN \"AS IS\"\n", stdout);
    fputs("    BASIS, WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED,\n", stdout);
    fputs("    INCLUDING, WITHOUT LIMITATION, WARRANTIES THAT THE COVERED\n", stdout);
    fputs("    SOFTWARE IS FREE OF DEFECTS, MERCHANTABLE, FIT FOR A PARTICULAR\n", stdout);
    fputs("    PURPOSE OR NON-INFRINGING.  THE ENTIRE RISK AS TO THE QUALITY AND\n", stdout);
    fputs("    PERFORMANCE OF THE COVERED SOFTWARE IS WITH YOU.  SHOULD ANY\n", stdout);
    fputs("    COVERED SOFTWARE PROVE DEFECTIVE IN ANY RESPECT, YOU (NOT THE\n", stdout);
    fputs("    INITIAL DEVELOPER OR ANY OTHER CONTRIBUTOR) ASSUME THE COST OF ANY\n", stdout);
    fputs("    NECESSARY SERVICING, REPAIR OR CORRECTION.  THIS DISCLAIMER OF\n", stdout);
    fputs("    WARRANTY CONSTITUTES AN ESSENTIAL PART OF THIS LICENSE.  NO USE OF\n", stdout);
    fputs("    ANY COVERED SOFTWARE IS AUTHORIZED HEREUNDER EXCEPT UNDER THIS\n", stdout);
    fputs("    DISCLAIMER.\n", stdout);
    fputs("\n", stdout);
    fputs("6. TERMINATION.\n", stdout);
    fputs("\n", stdout);
    fputs("    6.1. This License and the rights granted hereunder will terminate\n", stdout);
    fputs("    automatically if You fail to comply with terms herein and fail to\n", stdout);
    fputs("    cure such breach within 30 days of becoming aware of the breach.\n", stdout);
    fputs("    Provisions which, by their nature, must remain in effect beyond\n", stdout);
    fputs("    the termination of this License shall survive.\n", stdout);
    fputs("\n", stdout);
    fputs("    6.2. If You assert a patent infringement claim (excluding\n", stdout);
    fputs("    declaratory judgment actions) against Initial Developer or a\n", stdout);
    fputs("    Contributor (the Initial Developer or Contributor against whom You\n", stdout);
    fputs("    assert such claim is referred to as \"Participant\") alleging that\n", stdout);
    fputs("    the Participant Software (meaning the Contributor Version where\n", stdout);
    fputs("    the Participant is a Contributor or the Original Software where\n", stdout);
    fputs("    the Participant is the Initial Developer) directly or indirectly\n", stdout);
    fputs("    infringes any patent, then any and all rights granted directly or\n", stdout);
    fputs("    indirectly to You by such Participant, the Initial Developer (if\n", stdout);
    fputs("    the Initial Developer is not the Participant) and all Contributors\n", stdout);
    fputs("    under Sections 2.1 and/or 2.2 of this License shall, upon 60 days\n", stdout);
    fputs("    notice from Participant terminate prospectively and automatically\n", stdout);
    fputs("    at the expiration of such 60 day notice period, unless if within\n", stdout);
    fputs("    such 60 day period You withdraw Your claim with respect to the\n", stdout);
    fputs("    Participant Software against such Participant either unilaterally\n", stdout);
    fputs("    or pursuant to a written agreement with Participant.\n", stdout);
    fputs("\n", stdout);
    fputs("    6.3. In the event of termination under Sections 6.1 or 6.2 above,\n", stdout);
    fputs("    all end user licenses that have been validly granted by You or any\n", stdout);
    fputs("    distributor hereunder prior to termination (excluding licenses\n", stdout);
    fputs("    granted to You by any distributor) shall survive termination.\n", stdout);
    fputs("\n", stdout);
    fputs("7. LIMITATION OF LIABILITY.\n", stdout);
    fputs("\n", stdout);
    fputs("    UNDER NO CIRCUMSTANCES AND UNDER NO LEGAL THEORY, WHETHER TORT\n", stdout);
    fputs("    (INCLUDING NEGLIGENCE), CONTRACT, OR OTHERWISE, SHALL YOU, THE\n", stdout);
    fputs("    INITIAL DEVELOPER, ANY OTHER CONTRIBUTOR, OR ANY DISTRIBUTOR OF\n", stdout);
    fputs("    COVERED SOFTWARE, OR ANY SUPPLIER OF ANY OF SUCH PARTIES, BE\n", stdout);
    fputs("    LIABLE TO ANY PERSON FOR ANY INDIRECT, SPECIAL, INCIDENTAL, OR\n", stdout);
    fputs("    CONSEQUENTIAL DAMAGES OF ANY CHARACTER INCLUDING, WITHOUT\n", stdout);
    fputs("    LIMITATION, DAMAGES FOR LOST PROFITS, LOSS OF GOODWILL, WORK\n", stdout);
    fputs("    STOPPAGE, COMPUTER FAILURE OR MALFUNCTION, OR ANY AND ALL OTHER\n", stdout);
    fputs("    COMMERCIAL DAMAGES OR LOSSES, EVEN IF SUCH PARTY SHALL HAVE BEEN\n", stdout);
    fputs("    INFORMED OF THE POSSIBILITY OF SUCH DAMAGES.  THIS LIMITATION OF\n", stdout);
    fputs("    LIABILITY SHALL NOT APPLY TO LIABILITY FOR DEATH OR PERSONAL\n", stdout);
    fputs("    INJURY RESULTING FROM SUCH PARTY'S NEGLIGENCE TO THE EXTENT\n", stdout);
    fputs("    APPLICABLE LAW PROHIBITS SUCH LIMITATION.  SOME JURISDICTIONS DO\n", stdout);
    fputs("    NOT ALLOW THE EXCLUSION OR LIMITATION OF INCIDENTAL OR\n", stdout);
    fputs("    CONSEQUENTIAL DAMAGES, SO THIS EXCLUSION AND LIMITATION MAY NOT\n", stdout);
    fputs("    APPLY TO YOU.\n", stdout);
    fputs("\n", stdout);
    fputs("8. U.S. GOVERNMENT END USERS.\n", stdout);
    fputs("\n", stdout);
    fputs("    The Covered Software is a \"commercial item,\" as that term is\n", stdout);
    fputs("    defined in 48 C.F.R. 2.101 (Oct. 1995), consisting of \"commercial\n", stdout);
    fputs("    computer software\" (as that term is defined at 48\n", stdout);
    fputs("    C.F.R. 252.227-7014(a)(1)) and \"commercial computer software\n", stdout);
    fputs("    documentation\" as such terms are used in 48 C.F.R. 12.212\n", stdout);
    fputs("    (Sept. 1995).  Consistent with 48 C.F.R. 12.212 and 48\n", stdout);
    fputs("    C.F.R. 227.7202-1 through 227.7202-4 (June 1995), all\n", stdout);
    fputs("    U.S. Government End Users acquire Covered Software with only those\n", stdout);
    fputs("    rights set forth herein.  This U.S. Government Rights clause is in\n", stdout);
    fputs("    lieu of, and supersedes, any other FAR, DFAR, or other clause or\n", stdout);
    fputs("    provision that addresses Government rights in computer software\n", stdout);
    fputs("    under this License.\n", stdout);
    fputs("\n", stdout);
    fputs("9. MISCELLANEOUS.\n", stdout);
    fputs("\n", stdout);
    fputs("    This License represents the complete agreement concerning subject\n", stdout);
    fputs("    matter hereof.  If any provision of this License is held to be\n", stdout);
    fputs("    unenforceable, such provision shall be reformed only to the extent\n", stdout);
    fputs("    necessary to make it enforceable.  This License shall be governed\n", stdout);
    fputs("    by the law of the jurisdiction specified in a notice contained\n", stdout);
    fputs("    within the Original Software (except to the extent applicable law,\n", stdout);
    fputs("    if any, provides otherwise), excluding such jurisdiction's\n", stdout);
    fputs("    conflict-of-law provisions.  Any litigation relating to this\n", stdout);
    fputs("    License shall be subject to the jurisdiction of the courts located\n", stdout);
    fputs("    in the jurisdiction and venue specified in a notice contained\n", stdout);
    fputs("    within the Original Software, with the losing party responsible\n", stdout);
    fputs("    for costs, including, without limitation, court costs and\n", stdout);
    fputs("    reasonable attorneys' fees and expenses.  The application of the\n", stdout);
    fputs("    United Nations Convention on Contracts for the International Sale\n", stdout);
    fputs("    of Goods is expressly excluded.  Any law or regulation which\n", stdout);
    fputs("    provides that the language of a contract shall be construed\n", stdout);
    fputs("    against the drafter shall not apply to this License.  You agree\n", stdout);
    fputs("    that You alone are responsible for compliance with the United\n", stdout);
    fputs("    States export administration regulations (and the export control\n", stdout);
    fputs("    laws and regulation of any other countries) when You use,\n", stdout);
    fputs("    distribute or otherwise make available any Covered Software.\n", stdout);
    fputs("\n", stdout);
    fputs("10. RESPONSIBILITY FOR CLAIMS.\n", stdout);
    fputs("\n", stdout);
    fputs("    As between Initial Developer and the Contributors, each party is\n", stdout);
    fputs("    responsible for claims and damages arising, directly or\n", stdout);
    fputs("    indirectly, out of its utilization of rights under this License\n", stdout);
    fputs("    and You agree to work with Initial Developer and Contributors to\n", stdout);
    fputs("    distribute such responsibility on an equitable basis.  Nothing\n", stdout);
    fputs("    herein is intended or shall be deemed to constitute any admission\n", stdout);
    fputs("    of liability.\n", stdout);
    fputs("\n", stdout);
    fputs("--------------------------------------------------------------------\n", stdout);
    fputs("\n", stdout);
    fputs("NOTICE PURSUANT TO SECTION 9 OF THE COMMON DEVELOPMENT AND\n", stdout);
    fputs("DISTRIBUTION LICENSE (CDDL)\n", stdout);
    fputs("\n", stdout);
    fputs("For Covered Software in this distribution, this License shall\n", stdout);
    fputs("be governed by the laws of the State of California (excluding\n", stdout);
    fputs("conflict-of-law provisions).\n", stdout);
    fputs("\n", stdout);
    fputs("Any litigation relating to this License shall be subject to the\n", stdout);
    fputs("jurisdiction of the Federal Courts of the Northern District of\n", stdout);
    fputs("California and the state courts of the State of California, with\n", stdout);
    fputs("venue lying in Santa Clara County, California.");
}
#    endif // __illumos__ || THIS_IS_ILLUMOS
#endif     //__sun

#if IS_GLIBC_VERSION(1, 0)
static void print_GNU_LGPL_License(void)
{
    fputs("===========================================================================\n", stdout);
    fputs("glibc (GNU libc)\n\n", stdout);
    fputs("GNU LESSER GENERAL PUBLIC LICENSE\n\n", stdout);

    fputs("Version 3, 29 June 2007\n\n", stdout);

    fputs("Copyright (C) 2007 Free Software Foundation, Inc. <http://fsf.org/>\n\n", stdout);

    fputs("Everyone is permitted to copy and distribute verbatim copies of this license\n", stdout);
    fputs("document, but changing it is not allowed.\n\n", stdout);

    fputs("This version of the GNU Lesser General Public License incorporates the terms\n", stdout);
    fputs("and conditions of version 3 of the GNU General Public License, supplemented\n", stdout);
    fputs("by the additional permissions listed below.\n\n", stdout);

    fputs("0. Additional Definitions.\n\n", stdout);

    fputs("As used herein, \"this License\" refers to version 3 of the GNU Lesser General\n", stdout);
    fputs("Public License, and the \"GNU GPL\" refers to version 3 of the GNU General\n", stdout);
    fputs("Public License.\n\n", stdout);

    fputs("\"The Library\" refers to a covered work governed by this License, other than\n", stdout);
    fputs("an Application or a Combined Work as defined below.\n\n", stdout);

    fputs("An \"Application\" is any work that makes use of an interface provided by the\n", stdout);
    fputs("Library, but which is not otherwise based on the Library. Defining a subclass\n", stdout);
    fputs("of a class defined by the Library is deemed a mode of using an interface\n", stdout);
    fputs("provided by the Library.\n\n", stdout);

    fputs("A \"Combined Work\" is a work produced by combining or linking an Application\n", stdout);
    fputs("with the Library. The particular version of the Library with which the\n", stdout);
    fputs("Combined Work was made is also called the \"Linked Version\".\n\n", stdout);

    fputs("The \"Minimal Corresponding Source\" for a Combined Work means the\n", stdout);
    fputs("Corresponding Source for the Combined Work, excluding any source code for\n", stdout);
    fputs("portions of the Combined Work that, considered in isolation, are based on the\n", stdout);
    fputs("Application, and not on the Linked Version.\n\n", stdout);

    fputs("The \"Corresponding Application Code\" for a Combined Work means the object\n", stdout);
    fputs("code and/or source code for the Application, including any data and utility\n", stdout);
    fputs("programs needed for reproducing the Combined Work from the Application, but\n", stdout);
    fputs("excluding the System Libraries of the Combined Work.\n\n", stdout);

    fputs("1. Exception to Section 3 of the GNU GPL.\n\n", stdout);

    fputs("You may convey a covered work under sections 3 and 4 of this License without\n", stdout);
    fputs("being bound by section 3 of the GNU GPL.\n\n", stdout);

    fputs("2. Conveying Modified Versions.\n\n", stdout);

    fputs("If you modify a copy of the Library, and, in your modifications, a facility\n", stdout);
    fputs("refers to a function or data to be supplied by an Application that uses the\n", stdout);
    fputs("facility (other than as an argument passed when the facility is invoked),\n", stdout);
    fputs("then you may convey a copy of the modified version:\n\n", stdout);

    fputs("* a) under this License, provided that you make a good faith effort to ensure\n", stdout);
    fputs("that, in the event an Application does not supply the function or data, the\n", stdout);
    fputs("facility still operates, and performs whatever part of its purpose remains\n", stdout);
    fputs("meaningful, or\n\n", stdout);

    fputs("* b) under the GNU GPL, with none of the additional permissions of this\n", stdout);
    fputs("License applicable to that copy.\n\n", stdout);

    fputs("3. Object Code Incorporating Material from Library Header Files.\n\n", stdout);

    fputs("The object code form of an Application may incorporate material from a header\n", stdout);
    fputs("file that is part of the Library. You may convey such object code under terms\n", stdout);
    fputs("of your choice, provided that, if the incorporated material is not limited to\n", stdout);
    fputs("numerical parameters, data structure layouts and accessors, or small macros,\n", stdout);
    fputs("inline functions and templates (ten or fewer lines in length), you do both of\n", stdout);
    fputs("the following:\n\n", stdout);

    fputs("* a) Give prominent notice with each copy of the object code that the Library\n", stdout);
    fputs("is used in it and that the Library and its use are covered by this License.\n\n", stdout);

    fputs("* b) Accompany the object code with a copy of the GNU GPL and this license\n", stdout);
    fputs("document.\n\n", stdout);

    fputs("4. Combined Works.\n\n", stdout);

    fputs("You may convey a Combined Work under terms of your choice that, taken\n", stdout);
    fputs("together, effectively do not restrict modification of the portions of the\n", stdout);
    fputs("Library contained in the Combined Work and reverse engineering for debugging\n", stdout);
    fputs("such modifications, if you also do each of the following:\n\n", stdout);

    fputs("* a) Give prominent notice with each copy of the Combined Work that the\n", stdout);
    fputs("Library is used in it and that the Library and its use are covered by this\n", stdout);
    fputs("License.\n\n", stdout);

    fputs("* b) Accompany the Combined Work with a copy of the GNU GPL and this license\n", stdout);
    fputs("document.\n\n", stdout);

    fputs("* c) For a Combined Work that displays copyright notices during execution,\n", stdout);
    fputs("include the copyright notice for the Library among these notices, as well as\n", stdout);
    fputs("a reference directing the user to the copies of the GNU GPL and this license\n", stdout);
    fputs("document.\n\n", stdout);

    fputs("* d) Do one of the following:\n", stdout);
    fputs("  o 0) Convey the Minimal Corresponding Source under the terms of this\n", stdout);
    fputs("License, and the Corresponding Application Code in a form suitable for, and\n", stdout);
    fputs("under terms that permit, the user to recombine or relink the Application\n", stdout);
    fputs("with a modified version of the Linked Version to produce a modified\n", stdout);
    fputs("Combined Work, in the manner specified by section 6 of the GNU GPL for\n", stdout);
    fputs("conveying Corresponding Source.\n", stdout);
    fputs("  o 1) Use a suitable shared library mechanism for linking with the Library.\n", stdout);
    fputs("A suitable mechanism is one that (a) uses at run time a copy of the Library\n", stdout);
    fputs("already present on the user's computer system, and (b) will operate\n", stdout);
    fputs("properly with a modified version of the Library that is\n", stdout);
    fputs("interface-compatible with the Linked Version.\n\n", stdout);

    fputs("* e) Provide Installation Information, but only if you would otherwise be\n", stdout);
    fputs("required to provide such information under section 6 of the GNU GPL, and only\n", stdout);
    fputs("to the extent that such information is necessary to install and execute a\n", stdout);
    fputs("modified version of the Combined Work produced by recombining or relinking\n", stdout);
    fputs("the Application with a modified version of the Linked Version. (If you use\n", stdout);
    fputs("option 4d0, the Installation Information must accompany the Minimal\n", stdout);
    fputs("Corresponding Source and Corresponding Application Code. If you use option\n", stdout);
    fputs("4d1, you must provide the Installation Information in the manner specified by\n", stdout);
    fputs("section 6 of the GNU GPL for conveying Corresponding Source.)\n\n", stdout);

    fputs("5. Combined Libraries.\n\n", stdout);

    fputs("You may place library facilities that are a work based on the Library side by\n", stdout);
    fputs("side in a single library together with other library facilities that are not\n", stdout);
    fputs("Applications and are not covered by this License, and convey such a combined\n", stdout);
    fputs("library under terms of your choice, if you do both of the following:\n\n", stdout);

    fputs("* a) Accompany the combined library with a copy of the same work based on the\n", stdout);
    fputs("Library, uncombined with any other library facilities, conveyed under the\n", stdout);
    fputs("terms of this License.\n\n", stdout);

    fputs("* b) Give prominent notice with the combined library that part of it is a\n", stdout);
    fputs("work based on the Library, and explaining where to find the accompanying\n", stdout);
    fputs("uncombined form of the same work.\n\n", stdout);

    fputs("6. Revised Versions of the GNU Lesser General Public License.\n\n", stdout);

    fputs("The Free Software Foundation may publish revised and/or new versions of the\n", stdout);
    fputs("GNU Lesser General Public License from time to time. Such new versions will\n", stdout);
    fputs("be similar in spirit to the present version, but may differ in detail to\n", stdout);
    fputs("address new problems or concerns.\n\n", stdout);

    fputs("Each version is given a distinguishing version number. If the Library as you\n", stdout);
    fputs("received it specifies that a certain numbered version of the GNU Lesser\n", stdout);
    fputs("General Public License \"or any later version\" applies to it, you have the\n", stdout);
    fputs("option of following the terms and conditions either of that published version\n", stdout);
    fputs("or of any later version published by the Free Software Foundation. If the\n", stdout);
    fputs("Library as you received it does not specify a version number of the GNU\n", stdout);
    fputs("Lesser General Public License, you may choose any version of the GNU Lesser\n", stdout);
    fputs("General Public License ever published by the Free Software Foundation.\n\n", stdout);

    fputs("If the Library as you received it specifies that a proxy can decide whether\n", stdout);
    fputs("future versions of the GNU Lesser General Public License shall apply, that\n", stdout);
    fputs("proxy's public statement of acceptance of any version is permanent\n", stdout);
    fputs("authorization for you to choose that version for the Library.\n\n", stdout);
}
#endif // IS_GLIBC_VERSION(1, 0)

#if defined(USING_MUSL_LIBC) && USING_MUSL_LIBC > 0
static void print_Musl_MIT_License(void)
{
    fputs("===========================================================================\n", stdout);
    fputs("musl libc\n\n", stdout);
    fputs("Copyright (C) 2005-2020 Rich Felker, et al.\n"
          "\n"
          "Permission is hereby granted, free of charge, to any person obtaining\n"
          "a copy of this software and associated documentation files (the\n"
          "\"Software\"), to deal in the Software without restriction, including\n"
          "without limitation the rights to use, copy, modify, merge, publish,\n"
          "distribute, sublicense, and/or sell copies of the Software, and to\n"
          "permit persons to whom the Software is furnished to do so, subject to\n"
          "the following conditions:\n"
          "\n"
          "The above copyright notice and this permission notice shall be\n"
          "included in all copies or substantial portions of the Software.\n"
          "\n"
          "THE SOFTWARE IS PROVIDED \"AS IS\", WITHOUT WARRANTY OF ANY KIND,\n"
          "EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF\n"
          "MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.\n"
          "IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY\n"
          "CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,\n"
          "TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE\n"
          "SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.\n\n",
          stdout);
}
#endif // USING_MUSL_LIBC

#if defined(_WIN32)
// TODO: This is technically flagged with ENABLE_OFNVME in opensea-transport, but that is always on right now.
//       May need a way to access that flag to determine when this should or should-not be part of the license
//       output.-TJE
static void print_Open_Fabrics_NVMe_IOCTL_License(void)
{
    fputs("===========================================================================\n", stdout);
    fputs("open fabrics NVMe IOCTL\n\n", stdout);
    fputs("Copyright (c) 2011-2012                                                  \n", stdout);
    fputs("                                                                         \n", stdout);
    fputs("  Integrated Device Technology, Inc.                                     \n", stdout);
    fputs("  Intel Corporation                                                      \n", stdout);
    fputs("  LSI Corporation                                                        \n", stdout);
    fputs("                                                                         \n", stdout);
    fputs("All rights reserved.                                                     \n", stdout);
    fputs("                                                                         \n", stdout);
    fputs("*************************************************************************\n", stdout);
    fputs("                                                                         \n", stdout);
    fputs("Redistribution and use in source and binary forms, with or without       \n", stdout);
    fputs("modification, are permitted provided that the following conditions are   \n", stdout);
    fputs("met:                                                                     \n", stdout);
    fputs("                                                                         \n", stdout);
    fputs("  1. Redistributions of source code must retain the above copyright      \n", stdout);
    fputs("     notice, this list of conditions and the following disclaimer.       \n", stdout);
    fputs("                                                                         \n", stdout);
    fputs("  2. Redistributions in binary form must reproduce the above copyright   \n", stdout);
    fputs("     notice, this list of conditions and the following disclaimer in the \n", stdout);
    fputs("     documentation and/or other materials provided with the distribution.\n", stdout);
    fputs("                                                                         \n", stdout);
    fputs("THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS \"AS  \n", stdout);
    fputs("IS\" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,\n", stdout);
    fputs("THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR   \n", stdout);
    fputs("PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR        \n", stdout);
    fputs("CONTRIBUTORS BE LIABLE FOR ANY DIRECT,INDIRECT, INCIDENTAL, SPECIAL,     \n", stdout);
    fputs("EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,      \n", stdout);
    fputs("PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR       \n", stdout);
    fputs("PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF   \n", stdout);
    fputs("LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING     \n", stdout);
    fputs("NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS       \n", stdout);
    fputs("SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.             \n", stdout);
    fputs("                                                                         \n", stdout);
    fputs("The views and conclusions contained in the software and documentation    \n", stdout);
    fputs("are those of the authors and should not be interpreted as representing   \n", stdout);
    fputs("official policies, either expressed or implied, of Intel Corporation,    \n", stdout);
    fputs("Integrated Device Technology Inc., or Sandforce Corporation.             \n", stdout);
    fputs("\n", stdout);
}
#endif //_WIN32

void print_Open_Source_Licenses(void)
{
    // show this license for the getopt parser in all builds now since it is the getopt used under all OSs. Name should
    // be changed to portable-getopt or something in the future.-TJE
    print_Win_Getopt_Licenses();
#if defined(_WIN32)
    // TODO: This is technically flagged with ENABLE_OFNVME in opensea-transport, but that is always on right now.
    //       May need a way to access that flag to determine when this should or should-not be part of the license
    //       output.-TJE
    print_Open_Fabrics_NVMe_IOCTL_License();
#elif defined(__DragonFly__)
    print_DragonFlyBSD_License();
#elif defined(__FreeBSD__)
    print_FreeBSD_License();
#elif defined(__OpenBSD__)
    print_OpenBSD_License();
#elif defined(__NetBSD__)
    print_NetBSD_License();
#elif defined(__linux__)
#    if IS_GLIBC_VERSION(1, 0)
    // in other 'nix systems, we need to show this since we are using gnu libc
    print_GNU_LGPL_License();
#    else
#        if defined(USING_MUSL_LIBC) && USING_MUSL_LIBC > 0
    print_Musl_MIT_License();
#        else
// NOTE: This should work with gcc and clang to emit a warning. If this causes problems with other
//       compilers, using #pramga message may also work.
#            pragma GCC warning "Unknown libc license. Please specify a libc license. Ex: USING_MUSL_LIBC"
#        endif
#    endif
#elif defined(__sun)
#    if defined(__illumos__) || defined(THIS_IS_ILLUMOS)
    print_CDDL_License();
#    endif // __illumos__ || THIS_IS_ILLUMOS
#elif defined(_AIX)
    // Any special license for system libc/etc that needs to be shown. Cannot easily identify one at this time - TJE
#else
#    error "Please update #if for system library licenses!"
#endif
    fputs("===========================================================================\n\n", stdout);
}
