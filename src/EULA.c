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
//

// \file EULA.c
// \brief This file defines the a function to print the EULA for the license option in a tool
// BR note: http://tomeko.net/online_tools/cpp_text_escape.php?lang=en
// BR note: added print_wingetopt_EULA_To_Screen, print_mbedtls_zlib_EULA_To_Screen functions (22-Aug-2016)
// BR note: added Modifications notice to mgedtls license function (23-Aug-2016)
// BR note: openSeaChest project released under Mozilla Public License Version 2.0 (01-Nov-2017)

#include "EULA.h"

void print_EULA_To_Screen(int showApacheLicense, int showZlibLicense)
{
    printf("BINARIES and SOURCE CODE files of the openSeaChest open source project have\n");
    printf("been made available to you under the Mozilla Public License 2.0 (MPL).  Mozilla\n");
    printf("is the custodian of the Mozilla Public License (\"MPL\"), an open source/free\n");
    printf("software license.\n\n");

    printf("https://www.mozilla.org/en-US/MPL/\n\n");

    printf("Mozilla Public License Version 2.0\n");
    printf("==================================\n\n");

    printf("1. Definitions\n");
    printf("--------------\n\n");

    printf("1.1. \"Contributor\"\n");
    printf("    means each individual or legal entity that creates, contributes to\n");
    printf("    the creation of, or owns Covered Software.\n\n");

    printf("1.2. \"Contributor Version\"\n");
    printf("    means the combination of the Contributions of others (if any) used\n");
    printf("    by a Contributor and that particular Contributor's Contribution.\n\n");

    printf("1.3. \"Contribution\"\n");
    printf("    means Covered Software of a particular Contributor.\n\n");

    printf("1.4. \"Covered Software\"\n");
    printf("    means Source Code Form to which the initial Contributor has attached\n");
    printf("    the notice in Exhibit A, the Executable Form of such Source Code\n");
    printf("    Form, and Modifications of such Source Code Form, in each case\n");
    printf("    including portions thereof.\n\n");

    printf("1.5. \"Incompatible With Secondary Licenses\"\n");
    printf("    means\n\n");

    printf("    (a) that the initial Contributor has attached the notice described\n");
    printf("        in Exhibit B to the Covered Software; or\n\n");

    printf("    (b) that the Covered Software was made available under the terms of\n");
    printf("        version 1.1 or earlier of the License, but not also under the\n");
    printf("        terms of a Secondary License.\n\n");

    printf("1.6. \"Executable Form\"\n");
    printf("    means any form of the work other than Source Code Form.\n\n");

    printf("1.7. \"Larger Work\"\n");
    printf("    means a work that combines Covered Software with other material, in\n");
    printf("    a separate file or files, that is not Covered Software.\n\n");

    printf("1.8. \"License\"\n");
    printf("    means this document.\n\n");

    printf("1.9. \"Licensable\"\n");
    printf("    means having the right to grant, to the maximum extent possible,\n");
    printf("    whether at the time of the initial grant or subsequently, any and\n");
    printf("    all of the rights conveyed by this License.\n\n");

    printf("1.10. \"Modifications\"\n");
    printf("    means any of the following:\n\n");

    printf("    (a) any file in Source Code Form that results from an addition to,\n");
    printf("        deletion from, or modification of the contents of Covered\n");
    printf("        Software; or\n\n");

    printf("    (b) any new file in Source Code Form that contains any Covered\n");
    printf("        Software.\n\n");

    printf("1.11. \"Patent Claims\" of a Contributor\n");
    printf("    means any patent claim(s), including without limitation, method,\n");
    printf("    process, and apparatus claims, in any patent Licensable by such\n");
    printf("    Contributor that would be infringed, but for the grant of the\n");
    printf("    License, by the making, using, selling, offering for sale, having\n");
    printf("    made, import, or transfer of either its Contributions or its\n");
    printf("    Contributor Version.\n\n");

    printf("1.12. \"Secondary License\"\n");
    printf("    means either the GNU General Public License, Version 2.0, the GNU\n");
    printf("    Lesser General Public License, Version 2.1, the GNU Affero General\n");
    printf("    Public License, Version 3.0, or any later versions of those\n");
    printf("    licenses.\n\n");

    printf("1.13. \"Source Code Form\"\n");
    printf("    means the form of the work preferred for making modifications.\n\n");

    printf("1.14. \"You\" (or \"Your\")\n");
    printf("    means an individual or a legal entity exercising rights under this\n");
    printf("    License. For legal entities, \"You\" includes any entity that\n");
    printf("    controls, is controlled by, or is under common control with You. For\n");
    printf("    purposes of this definition, \"control\" means (a) the power, direct\n");
    printf("    or indirect, to cause the direction or management of such entity,\n");
    printf("    whether by contract or otherwise, or (b) ownership of more than\n");
    printf("    fifty percent (50%%) of the outstanding shares or beneficial\n");
    printf("    ownership of such entity.\n\n");

    printf("2. License Grants and Conditions\n");
    printf("--------------------------------\n\n");

    printf("2.1. Grants\n\n");

    printf("Each Contributor hereby grants You a world-wide, royalty-free,\n");
    printf("non-exclusive license:\n\n");

    printf("(a) under intellectual property rights (other than patent or trademark)\n");
    printf("    Licensable by such Contributor to use, reproduce, make available,\n");
    printf("    modify, display, perform, distribute, and otherwise exploit its\n");
    printf("    Contributions, either on an unmodified basis, with Modifications, or\n");
    printf("    as part of a Larger Work; and\n\n");

    printf("(b) under Patent Claims of such Contributor to make, use, sell, offer\n");
    printf("    for sale, have made, import, and otherwise transfer either its\n");
    printf("    Contributions or its Contributor Version.\n\n");

    printf("2.2. Effective Date\n\n");

    printf("The licenses granted in Section 2.1 with respect to any Contribution\n");
    printf("become effective for each Contribution on the date the Contributor first\n");
    printf("distributes such Contribution.\n\n");

    printf("2.3. Limitations on Grant Scope\n\n");

    printf("The licenses granted in this Section 2 are the only rights granted under\n");
    printf("this License. No additional rights or licenses will be implied from the\n");
    printf("distribution or licensing of Covered Software under this License.\n");
    printf("Notwithstanding Section 2.1(b) above, no patent license is granted by a\n");
    printf("Contributor:\n\n");

    printf("(a) for any code that a Contributor has removed from Covered Software;\n");
    printf("    or\n\n");

    printf("(b) for infringements caused by: (i) Your and any other third party's\n");
    printf("    modifications of Covered Software, or (ii) the combination of its\n");
    printf("    Contributions with other software (except as part of its Contributor\n");
    printf("    Version); or\n\n");

    printf("(c) under Patent Claims infringed by Covered Software in the absence of\n");
    printf("    its Contributions.\n\n");

    printf("This License does not grant any rights in the trademarks, service marks,\n");
    printf("or logos of any Contributor (except as may be necessary to comply with\n");
    printf("the notice requirements in Section 3.4).\n\n");

    printf("2.4. Subsequent Licenses\n\n");

    printf("No Contributor makes additional grants as a result of Your choice to\n");
    printf("distribute the Covered Software under a subsequent version of this\n");
    printf("License (see Section 10.2) or under the terms of a Secondary License (if\n");
    printf("permitted under the terms of Section 3.3).\n\n");

    printf("2.5. Representation\n\n");

    printf("Each Contributor represents that the Contributor believes its\n");
    printf("Contributions are its original creation(s) or it has sufficient rights\n");
    printf("to grant the rights to its Contributions conveyed by this License.\n\n");

    printf("2.6. Fair Use\n\n");

    printf("This License is not intended to limit any rights You have under\n");
    printf("applicable copyright doctrines of fair use, fair dealing, or other\n");
    printf("equivalents.\n\n");

    printf("2.7. Conditions\n\n");

    printf("Sections 3.1, 3.2, 3.3, and 3.4 are conditions of the licenses granted\n");
    printf("in Section 2.1.\n\n");

    printf("3. Responsibilities\n");
    printf("-------------------\n\n");

    printf("3.1. Distribution of Source Form\n\n");

    printf("All distribution of Covered Software in Source Code Form, including any\n");
    printf("Modifications that You create or to which You contribute, must be under\n");
    printf("the terms of this License. You must inform recipients that the Source\n");
    printf("Code Form of the Covered Software is governed by the terms of this\n");
    printf("License, and how they can obtain a copy of this License. You may not\n");
    printf("attempt to alter or restrict the recipients' rights in the Source Code\n");
    printf("Form.\n\n");

    printf("3.2. Distribution of Executable Form\n\n");

    printf("If You distribute Covered Software in Executable Form then:\n\n");

    printf("(a) such Covered Software must also be made available in Source Code\n");
    printf("    Form, as described in Section 3.1, and You must inform recipients of\n");
    printf("    the Executable Form how they can obtain a copy of such Source Code\n");
    printf("    Form by reasonable means in a timely manner, at a charge no more\n");
    printf("    than the cost of distribution to the recipient; and\n\n");

    printf("(b) You may distribute such Executable Form under the terms of this\n");
    printf("    License, or sublicense it under different terms, provided that the\n");
    printf("    license for the Executable Form does not attempt to limit or alter\n");
    printf("    the recipients' rights in the Source Code Form under this License.\n\n");

    printf("3.3. Distribution of a Larger Work\n\n");

    printf("You may create and distribute a Larger Work under terms of Your choice,\n");
    printf("provided that You also comply with the requirements of this License for\n");
    printf("the Covered Software. If the Larger Work is a combination of Covered\n");
    printf("Software with a work governed by one or more Secondary Licenses, and the\n");
    printf("Covered Software is not Incompatible With Secondary Licenses, this\n");
    printf("License permits You to additionally distribute such Covered Software\n");
    printf("under the terms of such Secondary License(s), so that the recipient of\n");
    printf("the Larger Work may, at their option, further distribute the Covered\n");
    printf("Software under the terms of either this License or such Secondary\n");
    printf("License(s).\n\n");

    printf("3.4. Notices\n\n");

    printf("You may not remove or alter the substance of any license notices\n");
    printf("(including copyright notices, patent notices, disclaimers of warranty,\n");
    printf("or limitations of liability) contained within the Source Code Form of\n");
    printf("the Covered Software, except that You may alter any license notices to\n");
    printf("the extent required to remedy known factual inaccuracies.\n\n");

    printf("3.5. Application of Additional Terms\n\n");

    printf("You may choose to offer, and to charge a fee for, warranty, support,\n");
    printf("indemnity or liability obligations to one or more recipients of Covered\n");
    printf("Software. However, You may do so only on Your own behalf, and not on\n");
    printf("behalf of any Contributor. You must make it absolutely clear that any\n");
    printf("such warranty, support, indemnity, or liability obligation is offered by\n");
    printf("You alone, and You hereby agree to indemnify every Contributor for any\n");
    printf("liability incurred by such Contributor as a result of warranty, support,\n");
    printf("indemnity or liability terms You offer. You may include additional\n");
    printf("disclaimers of warranty and limitations of liability specific to any\n");
    printf("jurisdiction.\n\n");

    printf("4. Inability to Comply Due to Statute or Regulation\n");
    printf("---------------------------------------------------\n\n");

    printf("If it is impossible for You to comply with any of the terms of this\n");
    printf("License with respect to some or all of the Covered Software due to\n");
    printf("statute, judicial order, or regulation then You must: (a) comply with\n");
    printf("the terms of this License to the maximum extent possible; and (b)\n");
    printf("describe the limitations and the code they affect. Such description must\n");
    printf("be placed in a text file included with all distributions of the Covered\n");
    printf("Software under this License. Except to the extent prohibited by statute\n");
    printf("or regulation, such description must be sufficiently detailed for a\n");
    printf("recipient of ordinary skill to be able to understand it.\n\n");

    printf("5. Termination\n");
    printf("--------------\n\n");

    printf("5.1. The rights granted under this License will terminate automatically\n");
    printf("if You fail to comply with any of its terms. However, if You become\n");
    printf("compliant, then the rights granted under this License from a particular\n");
    printf("Contributor are reinstated (a) provisionally, unless and until such\n");
    printf("Contributor explicitly and finally terminates Your grants, and (b) on an\n");
    printf("ongoing basis, if such Contributor fails to notify You of the\n");
    printf("non-compliance by some reasonable means prior to 60 days after You have\n");
    printf("come back into compliance. Moreover, Your grants from a particular\n");
    printf("Contributor are reinstated on an ongoing basis if such Contributor\n");
    printf("notifies You of the non-compliance by some reasonable means, this is the\n");
    printf("first time You have received notice of non-compliance with this License\n");
    printf("from such Contributor, and You become compliant prior to 30 days after\n");
    printf("Your receipt of the notice.\n\n");

    printf("5.2. If You initiate litigation against any entity by asserting a patent\n");
    printf("infringement claim (excluding declaratory judgment actions,\n");
    printf("counter-claims, and cross-claims) alleging that a Contributor Version\n");
    printf("directly or indirectly infringes any patent, then the rights granted to\n");
    printf("You by any and all Contributors for the Covered Software under Section\n");
    printf("2.1 of this License shall terminate.\n\n");

    printf("5.3. In the event of termination under Sections 5.1 or 5.2 above, all\n");
    printf("end user license agreements (excluding distributors and resellers) which\n");
    printf("have been validly granted by You or Your distributors under this License\n");
    printf("prior to termination shall survive termination.\n\n");

    printf("************************************************************************\n");
    printf("*                                                                      *\n");
    printf("*  6. Disclaimer of Warranty                                           *\n");
    printf("*  -------------------------                                           *\n");
    printf("*                                                                      *\n");
    printf("*  Covered Software is provided under this License on an \"as is\"       *\n");
    printf("*  basis, without warranty of any kind, either expressed, implied, or  *\n");
    printf("*  statutory, including, without limitation, warranties that the       *\n");
    printf("*  Covered Software is free of defects, merchantable, fit for a        *\n");
    printf("*  particular purpose or non-infringing. The entire risk as to the     *\n");
    printf("*  quality and performance of the Covered Software is with You.        *\n");
    printf("*  Should any Covered Software prove defective in any respect, You     *\n");
    printf("*  (not any Contributor) assume the cost of any necessary servicing,   *\n");
    printf("*  repair, or correction. This disclaimer of warranty constitutes an   *\n");
    printf("*  essential part of this License. No use of any Covered Software is   *\n");
    printf("*  authorized under this License except under this disclaimer.         *\n");
    printf("*                                                                      *\n");
    printf("************************************************************************\n\n");

    printf("************************************************************************\n");
    printf("*                                                                      *\n");
    printf("*  7. Limitation of Liability                                          *\n");
    printf("*  --------------------------                                          *\n");
    printf("*                                                                      *\n");
    printf("*  Under no circumstances and under no legal theory, whether tort      *\n");
    printf("*  (including negligence), contract, or otherwise, shall any           *\n");
    printf("*  Contributor, or anyone who distributes Covered Software as          *\n");
    printf("*  permitted above, be liable to You for any direct, indirect,         *\n");
    printf("*  special, incidental, or consequential damages of any character      *\n");
    printf("*  including, without limitation, damages for lost profits, loss of    *\n");
    printf("*  goodwill, work stoppage, computer failure or malfunction, or any    *\n");
    printf("*  and all other commercial damages or losses, even if such party      *\n");
    printf("*  shall have been informed of the possibility of such damages. This   *\n");
    printf("*  limitation of liability shall not apply to liability for death or   *\n");
    printf("*  personal injury resulting from such party's negligence to the       *\n");
    printf("*  extent applicable law prohibits such limitation. Some               *\n");
    printf("*  jurisdictions do not allow the exclusion or limitation of           *\n");
    printf("*  incidental or consequential damages, so this exclusion and          *\n");
    printf("*  limitation may not apply to You.                                    *\n");
    printf("*                                                                      *\n");
    printf("************************************************************************\n\n");

    printf("8. Litigation\n");
    printf("-------------\n\n");

    printf("Any litigation relating to this License may be brought only in the\n");
    printf("courts of a jurisdiction where the defendant maintains its principal\n");
    printf("place of business and such litigation shall be governed by laws of that\n");
    printf("jurisdiction, without reference to its conflict-of-law provisions.\n");
    printf("Nothing in this Section shall prevent a party's ability to bring\n");
    printf("cross-claims or counter-claims.\n\n");

    printf("9. Miscellaneous\n");
    printf("----------------\n\n");

    printf("This License represents the complete agreement concerning the subject\n");
    printf("matter hereof. If any provision of this License is held to be\n");
    printf("unenforceable, such provision shall be reformed only to the extent\n");
    printf("necessary to make it enforceable. Any law or regulation which provides\n");
    printf("that the language of a contract shall be construed against the drafter\n");
    printf("shall not be used to construe this License against a Contributor.\n\n");

    printf("10. Versions of the License\n");
    printf("---------------------------\n\n");

    printf("10.1. New Versions\n\n");

    printf("Mozilla Foundation is the license steward. Except as provided in Section\n");
    printf("10.3, no one other than the license steward has the right to modify or\n");
    printf("publish new versions of this License. Each version will be given a\n");
    printf("distinguishing version number.\n\n");

    printf("10.2. Effect of New Versions\n\n");

    printf("You may distribute the Covered Software under the terms of the version\n");
    printf("of the License under which You originally received the Covered Software,\n");
    printf("or under the terms of any subsequent version published by the license\n");
    printf("steward.\n\n");

    printf("10.3. Modified Versions\n\n");

    printf("If you create software not governed by this License, and you want to\n");
    printf("create a new license for such software, you may create and use a\n");
    printf("modified version of this License if you rename the license and remove\n");
    printf("any references to the name of the license steward (except to note that\n");
    printf("such modified license differs from this License).\n\n");

    printf("10.4. Distributing Source Code Form that is Incompatible With Secondary\n");
    printf("Licenses\n\n");

    printf("If You choose to distribute Source Code Form that is Incompatible With\n");
    printf("Secondary Licenses under the terms of this version of the License, the\n");
    printf("notice described in Exhibit B of this License must be attached.\n\n");

    printf("Exhibit A - Source Code Form License Notice\n");
    printf("-------------------------------------------\n\n");

    printf("  This Source Code Form is subject to the terms of the Mozilla Public\n");
    printf("  License, v. 2.0. If a copy of the MPL was not distributed with this\n");
    printf("  file, You can obtain one at http://mozilla.org/MPL/2.0/.\n\n");

    printf("If it is not possible or desirable to put the notice in a particular\n");
    printf("file, then You may include the notice in a location (such as a LICENSE\n");
    printf("file in a relevant directory) where a recipient would be likely to look\n");
    printf("for such a notice.\n\n");

    printf("You may add additional accurate notices of copyright ownership.\n\n");

    printf("Exhibit B - \"Incompatible With Secondary Licenses\" Notice\n");
    printf("---------------------------------------------------------\n\n");

    printf("  This Source Code Form is \"Incompatible With Secondary Licenses\", as\n");
    printf("  defined by the Mozilla Public License, v. 2.0.\n\n");
    print_Open_Source_Licenses(showApacheLicense, showZlibLicense);
    return;
}

void print_Apache_2_0_License()
{
    printf("===========================================================================\n");
    printf("mbedtls - An open source, portable, easy to use, readable and flexible SSL\n");
    printf("library https://tls.mbed.org\n\n");

    printf("Modifications: -added DES & 3DES CFB cipher encryption and decryption support\n\n");

    printf("         Apache License\n");
    printf("   Version 2.0, January 2004\n");
    printf("http://www.apache.org/licenses/\n\n");

    printf("TERMS AND CONDITIONS FOR USE, REPRODUCTION, AND DISTRIBUTION\n\n");

    printf("1. Definitions.\n\n");

    printf("\"License\" shall mean the terms and conditions for use, reproduction, and\n");
    printf("distribution as defined by Sections 1 through 9 of this document.\n\n");

    printf("\"Licensor\" shall mean the copyright owner or entity authorized by the copyright\n");
    printf("owner that is granting the License.\n\n");

    printf("\"Legal Entity\" shall mean the union of the acting entity and all other entities\n");
    printf("that control, are controlled by, or are under common control with that entity.\n");
    printf("For the purposes of this definition, \"control\" means (i) the power, direct or\n");
    printf("indirect, to cause the direction or management of such entity, whether by\n");
    printf("contract or otherwise, or (ii) ownership of fifty percent (50) or more of the\n");
    printf("outstanding shares, or (iii) beneficial ownership of such entity.\n\n");

    printf("\"You\" (or \"Your\") shall mean an individual or Legal Entity exercising\n");
    printf("permissions granted by this License.\n\n");

    printf("\"Source\" form shall mean the preferred form for making modifications, including\n");
    printf("but not limited to software source code, documentation source,and configuration\n");
    printf("files.\n\n");

    printf("\"Object\" form shall mean any form resulting from mechanical transformation or\n");
    printf("translation of a Source form, including but not limited to compiled object\n");
    printf("code, generated documentation, and conversions to other media types.\n\n");

    printf("\"Work\" shall mean the work of authorship, whether in Source or Object form,\n");
    printf("made available under the License, as indicated by a copyright notice that is\n");
    printf("included in or attached to the work (an example is provided in the Appendix\n");
    printf("below).\n\n");

    printf("\"Derivative Works\" shall mean any work, whether in Source or Object form, that\n");
    printf("is based on (or derived from) the Work and for which the editorial revisions,\n");
    printf("annotations, elaborations, or other modifications represent, as a whole, an\n");
    printf("original work of authorship. For the purposes of this License, Derivative Works\n");
    printf("shall not include works that remain separable from, or merely link (or bind by\n");
    printf("name) to the interfaces of, the Work and Derivative Works thereof.\n\n");

    printf("\"Contribution\" shall mean any work of authorship, including the original\n");
    printf("version of the Work and any modifications or additions to that Work or\n");
    printf("Derivative Works thereof, that is intentionally submitted to Licensor for\n");
    printf("inclusion in the Work by the copyright owner or by an individual or Legal\n");
    printf("Entity authorized to submit on behalf of the copyright owner. For the purposes\n");
    printf("of this definition, \"submitted\" means any form of electronic, verbal, or\n");
    printf("written communication sent to the Licensor or its representatives, including\n");
    printf("but not limited to communication on electronic mailing lists, source code\n");
    printf("control systems, and issue tracking systems that are managed by, or on behalf\n");
    printf("of, the Licensor for the purpose of discussing and improving the Work, but\n");
    printf("excluding communication that is conspicuously marked or otherwise designated in\n");
    printf("writing by the copyright owner as \"Not a Contribution.\"\n\n");

    printf("\"Contributor\" shall mean Licensor and any individual or Legal Entity on behalf\n");
    printf("of whom a Contribution has been received by Licensor and subsequently\n");
    printf("incorporated within the Work.\n\n");

    printf("2. Grant of Copyright License. Subject to the terms and conditions of this\n");
    printf("License, each Contributor hereby grants to You a perpetual, worldwide,\n");
    printf("non-exclusive, no-charge, royalty-free, irrevocable copyright license to\n");
    printf("reproduce, prepare Derivative Works of, publicly display, publicly perform,\n");
    printf("sublicense, and distribute the Work and such Derivative Works in Source or\n");
    printf("Object form.\n\n");

    printf("3. Grant of Patent License. Subject to the terms and conditions of this\n");
    printf("License, each Contributor hereby grants to You a perpetual, worldwide,\n");
    printf("non-exclusive, no-charge, royalty-free, irrevocable (except as stated in this\n");
    printf("section) patent license to make, have made, use, offer to sell, sell, import,\n");
    printf("and otherwise transfer the Work, where such license applies only to those\n");
    printf("patent claims licensable by such Contributor that are necessarily infringed by\n");
    printf("their Contribution(s) alone or by combination of their Contribution(s) with the\n");
    printf("Work to which such Contribution(s) was submitted. If You institute patent\n");
    printf("litigation against any entity (including a cross-claim or counterclaim in a\n");
    printf("lawsuit) alleging that the Work or a Contribution incorporated within the Work\n");
    printf("constitutes direct or contributory patent infringement, then any patent\n");
    printf("licenses granted to You under this License for that Work shall terminate as of\n");
    printf("the date such litigation is filed.\n\n");

    printf("4. Redistribution. You may reproduce and distribute copies of the Work or\n");
    printf("Derivative Works thereof in any medium, with or without modifications, and in\n");
    printf("Source or Object form, provided that You meet the following conditions:\n\n");

    printf("(a) You must give any other recipients of the Work or Derivative Works a copy\n");
    printf("of this License; and\n\n");

    printf("(b) You must cause any modified files to carry prominent notices stating that\n");
    printf("You changed the files; and\n\n");

    printf("(c) You must retain, in the Source form of any Derivative Works that You\n");
    printf("distribute, all copyright, patent, trademark, and attribution notices from the\n");
    printf("Source form of the Work, excluding those notices that do not pertain to any\n");
    printf("part of the Derivative Works; and\n\n");

    printf("(d) If the Work includes a \"NOTICE\" text file as part of its distribution, then\n");
    printf("any Derivative Works that You distribute must include a readable copy of the\n");
    printf("attribution notices contained within such NOTICE file, excluding those notices\n");
    printf("that do not pertain to any part of the Derivative Works, in at least one of the\n");
    printf("following places: within a NOTICE text file distributed as part of the\n");
    printf("Derivative Works; within the Source form or documentation, if provided along\n");
    printf("with the Derivative Works; or, within a display generated by the Derivative\n");
    printf("Works, if and wherever such third-party notices normally appear. The contents\n");
    printf("of the NOTICE file are for informational purposes only and do not modify the\n");
    printf("License. You may add Your own attribution notices within Derivative Works that\n");
    printf("You distribute, alongside or as an addendum to the NOTICE text from the Work,\n");
    printf("provided that such additional attribution notices cannot be construed as\n");
    printf("modifying the License.\n\n");

    printf("You may add Your own copyright statement to Your modifications and may provide\n");
    printf("additional or different license terms and conditions for use, reproduction, or\n");
    printf("distribution of Your modifications, or for any such Derivative Works as a\n");
    printf("whole, provided Your use, reproduction, and distribution of the Work otherwise\n");
    printf("complies with the conditions stated in this License.\n\n");

    printf("5. Submission of Contributions. Unless You explicitly state otherwise, any\n");
    printf("Contribution intentionally submitted for inclusion in the Work by You to the\n");
    printf("Licensor shall be under the terms and conditions of this License, without any\n");
    printf("additional terms or conditions. Notwithstanding the above, nothing herein shall\n");
    printf("supersede or modify the terms of any separate license agreement you may have\n");
    printf("executed with Licensor regarding such Contributions.\n\n");

    printf("6. Trademarks. This License does not grant permission to use the trade names,\n");
    printf("trademarks, service marks, or product names of the Licensor, except as required\n");
    printf("for reasonable and customary use in describing the origin of the Work and\n");
    printf("reproducing the content of the NOTICE file.\n\n");

    printf("7. Disclaimer of Warranty. Unless required by applicable law or agreed to in\n");
    printf("writing, Licensor provides the Work (and each Contributor provides its\n");
    printf("Contributions) on an \"AS IS\" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY\n");
    printf("KIND, either express or implied, including, without limitation, any warranties\n");
    printf("or conditions of TITLE, NON-INFRINGEMENT, MERCHANTABILITY, or FITNESS FOR A\n");
    printf("PARTICULAR PURPOSE. You are solely responsible for determining the\n");
    printf("appropriateness of using or redistributing the Work and assume any risks\n");
    printf("associated with Your exercise of permissions under this License.\n\n");

    printf("8. Limitation of Liability. In no event and under no legal theory, whether in\n");
    printf("tort (including negligence), contract, or otherwise, unless required by\n");
    printf("applicable law (such as deliberate and grossly negligent acts) or agreed to in\n");
    printf("writing, shall any Contributor be liable to You for damages, including any\n");
    printf("direct, indirect, special, incidental, or consequential damages of any\n");
    printf("character arising as a result of this License or out of the use or inability to\n");
    printf("use the Work (including but not limited to damages for loss of goodwill, work\n");
    printf("stoppage, computer failure or malfunction, or any and all other commercial\n");
    printf("damages or losses), even if such Contributor has been advised of the\n");
    printf("possibility of such damages.\n\n");

    printf("9. Accepting Warranty or Additional Liability. While redistributing the Work or\n");
    printf("Derivative Works thereof, You may choose to offer, and charge a fee for,\n");
    printf("acceptance of support, warranty, indemnity, or other liability obligations\n");
    printf("and/or rights consistent with this License. However, in accepting such\n");
    printf("obligations, You may act only on Your own behalf and on Your sole\n");
    printf("responsibility, not on behalf of any other Contributor, and only if You agree\n");
    printf("to indemnify, defend, and hold each Contributor harmless for any liability\n");
    printf("incurred by, or claims asserted against, such Contributor by reason of your\n");
    printf("accepting any such warranty or additional liability.\n\n");

    printf("END OF TERMS AND CONDITIONS\n\n");

    printf("APPENDIX: How to apply the Apache License to your work.\n\n");

    printf("To apply the Apache License to your work, attach the following boilerplate\n");
    printf("notice, with the fields enclosed by brackets \"[]\" replaced with your own\n");
    printf("identifying information. (Don't include the brackets!) The text should be\n");
    printf("enclosed in the appropriate comment syntax for the file format. We also\n");
    printf("recommend that a file or class name and description of purpose be included on\n");
    printf("the same \"printed page\" as the copyright notice for easier identification\n");
    printf("within third-party archives.\n\n");

    printf("Copyright [yyyy] [name of copyright owner]\n\n");

    printf("Licensed under the Apache License, Version 2.0 (the \"License\"); you may not use\n");
    printf("this file except in compliance with the License. You may obtain a copy of the\n");
    printf("License at\n\n");

    printf("http://www.apache.org/licenses/LICENSE-2.0\n\n");

    printf("Unless required by applicable law or agreed to in writing, software distributed\n");
    printf("under the License is distributed on an \"AS IS\" BASIS, WITHOUT WARRANTIES OR\n");
    printf("CONDITIONS OF ANY KIND, either express or implied. See the License for the\n");
    printf("specific language governing permissions and limitations under the License.\n\n");
    return;
}

void print_Zlib_License()
{
    printf("zlib.h -- interface of the 'zlib' general purpose compression library version\n");
    printf("1.2.8, April 28th, 2013\n\n");

    printf("Copyright (C) 1995-2013 Jean-loup Gailly and Mark Adler\n\n");

    printf("This software is provided 'as-is', without any express or implied warranty. In\n");
    printf("no event will the authors be held liable for any damages arising from the use\n");
    printf("of this software.\n\n");

    printf("Permission is granted to anyone to use this software for any purpose, including\n");
    printf("commercial applications, and to alter it and redistribute it freely, subject to\n");
    printf("the following restrictions:\n\n");

    printf("1. The origin of this software must not be misrepresented; you must not claim\n");
    printf("that you wrote the original software. If you use this software in a product, an\n");
    printf("acknowledgment in the product documentation would be appreciated but is not\n");
    printf("required.\n\n");

    printf("2. Altered source versions must be plainly marked as such, and must not be\n");
    printf("misrepresented as being the original software.\n\n");

    printf("3. This notice may not be removed or altered from any source distribution.\n\n");

    printf("Jean-loup Gailly        Mark Adler jloup@gzip.org\n");
    printf("madler@alumni.caltech.edu\n\n");
    return;
}

#if defined (_WIN32)
static void print_Win_Getopt_Licenses()
{
    printf("===========================================================================\n");
    printf("wingetopt - getopt library for Windows compilers\n\n");

    printf("This library was created to allow compilation linux-based software on Windows.\n");
    printf("http://en.wikipedia.org/wiki/Getopt  The sources were taken from MinGW-runtime\n");
    printf("project.\n\n");

    printf("AUTHORS: Todd C. Miller Todd.Miller@courtesan.com; The NetBSD Foundation, Inc.\n\n");

    printf("LICENSE\n\n");

    printf("Copyright (c) 2002 Todd C. Miller <Todd.Miller@courtesan.com>\n\n");

    printf("Permission to use, copy, modify, and distribute this software for any purpose\n");
    printf("with or without fee is hereby granted, provided that the above copyright notice\n");
    printf("and this permission notice appear in all copies.\n\n");

    printf("THE SOFTWARE IS PROVIDED \"AS IS\" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH\n");
    printf("REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND\n");
    printf("FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,\n");
    printf("INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM\n");
    printf("LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR\n");
    printf("OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR\n");
    printf("PERFORMANCE OF THIS SOFTWARE.\n\n");

    printf("Sponsored in part by the Defense Advanced Research Projects Agency (DARPA) and\n");
    printf("Air Force Research Laboratory, Air Force Materiel Command, USAF, under\n");
    printf("agreement number F39502-99-1-0512.\n\n");

    printf("                 -------------------------------------\n\n");

    printf("Copyright (c) 2000 The NetBSD Foundation, Inc.\n");
    printf("All rights reserved.\n\n");

    printf("This code is derived from software contributed to The NetBSD Foundation\n");
    printf("by Dieter Baron and Thomas Klausner.\n\n");

    printf("Redistribution and use in source and binary forms, with or without\n");
    printf("modification, are permitted provided that the following conditions are met:\n");
    printf("1. Redistributions of source code must retain the above copyright notice, this\n");
    printf("   list of conditions and the following disclaimer.\n");
    printf("2. Redistributions in binary form must reproduce the above copyright notice,\n");
    printf("   this list of conditions and the following disclaimer in the documentation\n");
    printf("   and/or other materials provided with the distribution.\n\n");

    printf("THIS SOFTWARE IS PROVIDED BY THE NETBSD FOUNDATION, INC. AND CONTRIBUTORS \"AS\n");
    printf("IS\" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE\n");
    printf("IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE\n");
    printf("DISCLAIMED.  IN NO EVENT SHALL THE FOUNDATION OR CONTRIBUTORS BE LIABLE FOR ANY\n");
    printf("DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES\n");
    printf("(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;\n");
    printf("LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON\n");
    printf("ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT\n");
    printf("(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS\n");
    printf("SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.\n\n");
    return;
}
#endif//_WIN32

#if defined (__FreeBSD__)
static void print_FreeBSD_License()
{
    printf("Copyright 1992 - 2019 The FreeBSD Project.\n\n");
    printf("Redistribution and use in source and binary forms, with or without\n");
    printf("modification, are permitted provided that the following conditions are met :\n");
    printf("\n");
    printf("1. Redistributions of source code must retain the above copyright notice,\n");
    printf("this list of conditions and the following disclaimer.\n");
    printf("2. Redistributions in binary form must reproduce the above copyright notice,\n");
    printf("this list of conditions and the following disclaimer in the documentation\n");
    printf("and/or other materials provided with the distribution.\n");
    printf("\n");
    printf("THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND ANY\n");
    printf("EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED\n");
    printf("WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.\n");
    printf("IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,\n");
    printf("INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES(INCLUDING, BUT NOT\n");
    printf("LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,\n");
    printf("OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF\n");
    printf("LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT(INCLUDING NEGLIGENCE\n");
    printf("OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED\n");
    printf("OF THE POSSIBILITY OF SUCH DAMAGE.\n");
    printf("\n");
    printf("The views and conclusions contained in the software and documentation are those of\n");
    printf("the authors and should not be interpreted as representing official policies, either\n");
    printf("expressed or implied, of the FreeBSD Project.\n");
    printf("\n\n");
}
#endif //__FreeBSD__

#if defined(__GLIBC__)
static void print_GNU_LGPL_License()
{
    printf("===========================================================================\n");
    printf("glibc (GNU libc)\n\n");
    printf("GNU LESSER GENERAL PUBLIC LICENSE\n\n");

    printf("Version 3, 29 June 2007\n\n");

    printf("Copyright © 2007 Free Software Foundation, Inc. <http://fsf.org/>\n\n");

    printf("Everyone is permitted to copy and distribute verbatim copies of this license\n");
    printf("document, but changing it is not allowed.\n\n");

    printf("This version of the GNU Lesser General Public License incorporates the terms\n");
    printf("and conditions of version 3 of the GNU General Public License, supplemented\n");
    printf("by the additional permissions listed below.\n\n");

    printf("0. Additional Definitions.\n\n");

    printf("As used herein, \"this License\" refers to version 3 of the GNU Lesser General\n");
    printf("Public License, and the \"GNU GPL\" refers to version 3 of the GNU General\n");
    printf("Public License.\n\n");

    printf("\"The Library\" refers to a covered work governed by this License, other than\n");
    printf("an Application or a Combined Work as defined below.\n\n");

    printf("An \"Application\" is any work that makes use of an interface provided by the\n");
    printf("Library, but which is not otherwise based on the Library. Defining a subclass\n");
    printf("of a class defined by the Library is deemed a mode of using an interface\n");
    printf("provided by the Library.\n\n");

    printf("A \"Combined Work\" is a work produced by combining or linking an Application\n");
    printf("with the Library. The particular version of the Library with which the\n");
    printf("Combined Work was made is also called the \"Linked Version\".\n\n");

    printf("The \"Minimal Corresponding Source\" for a Combined Work means the\n");
    printf("Corresponding Source for the Combined Work, excluding any source code for\n");
    printf("portions of the Combined Work that, considered in isolation, are based on the\n");
    printf("Application, and not on the Linked Version.\n\n");

    printf("The \"Corresponding Application Code\" for a Combined Work means the object\n");
    printf("code and/or source code for the Application, including any data and utility\n");
    printf("programs needed for reproducing the Combined Work from the Application, but\n");
    printf("excluding the System Libraries of the Combined Work.\n\n");

    printf("1. Exception to Section 3 of the GNU GPL.\n\n");

    printf("You may convey a covered work under sections 3 and 4 of this License without\n");
    printf("being bound by section 3 of the GNU GPL.\n\n");

    printf("2. Conveying Modified Versions.\n\n");

    printf("If you modify a copy of the Library, and, in your modifications, a facility\n");
    printf("refers to a function or data to be supplied by an Application that uses the\n");
    printf("facility (other than as an argument passed when the facility is invoked),\n");
    printf("then you may convey a copy of the modified version:\n\n");

    printf("* a) under this License, provided that you make a good faith effort to ensure\n");
    printf("that, in the event an Application does not supply the function or data, the\n");
    printf("facility still operates, and performs whatever part of its purpose remains\n");
    printf("meaningful, or\n\n");

    printf("* b) under the GNU GPL, with none of the additional permissions of this\n");
    printf("License applicable to that copy.\n\n");

    printf("3. Object Code Incorporating Material from Library Header Files.\n\n");

    printf("The object code form of an Application may incorporate material from a header\n");
    printf("file that is part of the Library. You may convey such object code under terms\n");
    printf("of your choice, provided that, if the incorporated material is not limited to\n");
    printf("numerical parameters, data structure layouts and accessors, or small macros,\n");
    printf("inline functions and templates (ten or fewer lines in length), you do both of\n");
    printf("the following:\n\n");

    printf("* a) Give prominent notice with each copy of the object code that the Library\n");
    printf("is used in it and that the Library and its use are covered by this License.\n\n");

    printf("* b) Accompany the object code with a copy of the GNU GPL and this license\n");
    printf("document.\n\n");

    printf("4. Combined Works.\n\n");

    printf("You may convey a Combined Work under terms of your choice that, taken\n");
    printf("together, effectively do not restrict modification of the portions of the\n");
    printf("Library contained in the Combined Work and reverse engineering for debugging\n");
    printf("such modifications, if you also do each of the following:\n\n");

    printf("* a) Give prominent notice with each copy of the Combined Work that the\n");
    printf("Library is used in it and that the Library and its use are covered by this\n");
    printf("License.\n\n");

    printf("* b) Accompany the Combined Work with a copy of the GNU GPL and this license\n");
    printf("document.\n\n");

    printf("* c) For a Combined Work that displays copyright notices during execution,\n");
    printf("include the copyright notice for the Library among these notices, as well as\n");
    printf("a reference directing the user to the copies of the GNU GPL and this license\n");
    printf("document.\n\n");

    printf("* d) Do one of the following:\n");
    printf("  o 0) Convey the Minimal Corresponding Source under the terms of this\n");
    printf("License, and the Corresponding Application Code in a form suitable for, and\n");
    printf("under terms that permit, the user to recombine or relink the Application\n");
    printf("with a modified version of the Linked Version to produce a modified\n");
    printf("Combined Work, in the manner specified by section 6 of the GNU GPL for\n");
    printf("conveying Corresponding Source.\n");
    printf("  o 1) Use a suitable shared library mechanism for linking with the Library.\n");
    printf("A suitable mechanism is one that (a) uses at run time a copy of the Library\n");
    printf("already present on the user's computer system, and (b) will operate\n");
    printf("properly with a modified version of the Library that is\n");
    printf("interface-compatible with the Linked Version.\n\n");

    printf("* e) Provide Installation Information, but only if you would otherwise be\n");
    printf("required to provide such information under section 6 of the GNU GPL, and only\n");
    printf("to the extent that such information is necessary to install and execute a\n");
    printf("modified version of the Combined Work produced by recombining or relinking\n");
    printf("the Application with a modified version of the Linked Version. (If you use\n");
    printf("option 4d0, the Installation Information must accompany the Minimal\n");
    printf("Corresponding Source and Corresponding Application Code. If you use option\n");
    printf("4d1, you must provide the Installation Information in the manner specified by\n");
    printf("section 6 of the GNU GPL for conveying Corresponding Source.)\n\n");

    printf("5. Combined Libraries.\n\n");

    printf("You may place library facilities that are a work based on the Library side by\n");
    printf("side in a single library together with other library facilities that are not\n");
    printf("Applications and are not covered by this License, and convey such a combined\n");
    printf("library under terms of your choice, if you do both of the following:\n\n");

    printf("* a) Accompany the combined library with a copy of the same work based on the\n");
    printf("Library, uncombined with any other library facilities, conveyed under the\n");
    printf("terms of this License.\n\n");

    printf("* b) Give prominent notice with the combined library that part of it is a\n");
    printf("work based on the Library, and explaining where to find the accompanying\n");
    printf("uncombined form of the same work.\n\n");

    printf("6. Revised Versions of the GNU Lesser General Public License.\n\n");

    printf("The Free Software Foundation may publish revised and/or new versions of the\n");
    printf("GNU Lesser General Public License from time to time. Such new versions will\n");
    printf("be similar in spirit to the present version, but may differ in detail to\n");
    printf("address new problems or concerns.\n\n");

    printf("Each version is given a distinguishing version number. If the Library as you\n");
    printf("received it specifies that a certain numbered version of the GNU Lesser\n");
    printf("General Public License \"or any later version\" applies to it, you have the\n");
    printf("option of following the terms and conditions either of that published version\n");
    printf("or of any later version published by the Free Software Foundation. If the\n");
    printf("Library as you received it does not specify a version number of the GNU\n");
    printf("Lesser General Public License, you may choose any version of the GNU Lesser\n");
    printf("General Public License ever published by the Free Software Foundation.\n\n");

    printf("If the Library as you received it specifies that a proxy can decide whether\n");
    printf("future versions of the GNU Lesser General Public License shall apply, that\n");
    printf("proxy's public statement of acceptance of any version is permanent\n");
    printf("authorization for you to choose that version for the Library.\n\n");
    return;
}
#endif//__GLIBC__

#if defined (USING_MUSL_LIBC)
static void print_Musl_MIT_License()
{
    printf("===========================================================================\n");
    printf("musl libc\n\n");
    printf("Copyright © 2005-2020 Rich Felker, et al.\n"
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
    "SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.\n\n");
    return;
}
#endif //USING_MUSL_LIBC

void print_Open_Source_Licenses(int showApacheLicense, int showZlibLicense)
{
#if defined(_WIN32)
    //show this license for the getopt parser in windows
    print_Win_Getopt_Licenses();
#elif defined (__FreeBSD__)
    print_FreeBSD_License();
#elif defined (__linux__)
    #if defined (__GLIBC__)
        //in other 'nix systems, we need to show this since we are using gnu libc
        print_GNU_LGPL_License();
    #else
        #if defined (USING_MUSL_LIBC)
            print_Musl_MIT_License();
        #else
            //NOTE: This should work with gcc and clang to emit a warning. If this causes problems with other
            //      compilers, using #pramga message may also work.
            #pragma GCC diagnostic warning "Unknown libc license. Please specify a libc license. Ex: USING_MUSL_LIBC"
        #endif
    #endif
#elif defined (__sun)
    //TODO: Any special license for system libc/etc that needs to be shown. Cannot easily identify one at this time - TJE
#else
	#error Please update #if for system library licenses!
#endif
    printf("===========================================================================\n\n");
    if (showApacheLicense)
    {
        printf("===========================================================================\n\n");
        print_Apache_2_0_License();
    }
    if (showZlibLicense)
    {
        printf("===========================================================================\n\n");
        print_Zlib_License();
    }
    return;
}





