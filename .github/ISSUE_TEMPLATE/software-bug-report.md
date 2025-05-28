---
name: Software Bug Report
about: Report a bug for us to fix in openSeaChest
title: "[BUG] - Title"
labels: bug
assignees: vonericsen

---

**Describe the bug**
A clear and concise description of what the bug is.

**To Reproduce**
Steps to reproduce the behavior:
1. Go to '...'
2. Click on '....'
3. Scroll down to '....'
4. See error

**Expected behavior**
A clear and concise description of what you expected to happen.

**Screenshots**
If applicable, add screenshots to help explain your problem.

**Verbose Output**
Please attach the verbose output from the command you ran to help us review the raw drive responses.
For firmware updates or read/write operations, `-v3` is fine. For other operations, use `-v4`.
Ex: `openSeaChest_SMART -d <handle> -i -v4 | tee verboseInfo.txt`

**Desktop (please complete the following information):**
 - OS: [e.g. Windows/Linux]
 - Version [e.g. 22]

Hint: All `openSeaChest` tools support the `--version` option which can report this for you.

**Additional context**
Add any other context about the problem here that would be helpful to understand.
This can be a big picture overview of what you are trying to do or what background information led you to running these openSeaChest tools.
Does another tool run this same operation successfully? (e.g.smartctl, hdparm, sdparm, camcontrol, etc)
