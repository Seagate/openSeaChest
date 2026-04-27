---
name: "Firmware / Security Defect Intake (Data Collection)"
about: "Collect required openSeaChest diagnostics before submitting a firmware defect via Seagate's Responsible Disclosure process."
title: "[Firmware Defect]"
labels: ["bug", "Firmware"]
assignees: []
---

# Firmware Defect Intake Template  
_For reporting drive behavior that may indicate a firmware or security defect_

This template collects all information needed before submitting a firmware-related issue to Seagate.

openSeaChest **cannot** accept firmware defect reports directly and will automatically close such submissions.

Firmware defects may still require evaluation by Seagate engineering and, in many cases, may qualify as **security defects** (for example, if the drive becomes unresponsive or “bricked”).

To submit to Seagate, use:

🔗 https://www.seagate.com/legal/policies/responsible-vulnerability-disclosure-policy/

---

## 1. Purpose  

Some firmware anomalies represent security defects. Others are functional issues requiring engineering review.

This template ensures your report contains the correct data before submission through Seagate’s Responsible Vulnerability Disclosure process.

---

## 2. Reproduction Steps  

Provide exact, copy-pasteable steps to reliably reproduce the issue.

1.  
2.  
3.  

If intermittent, describe:
- Frequency  
- Conditions (thermal, workload, idle time, power cycles, etc.)

---

## 3. Environment Data  

### Drive Information  
- **Drive Model:**  
- **Drive Serial Number (required):**  
- **Firmware Version:**  
- **Interface:** SATA / SAS / NVMe  
- **Form Factor:**  

### Host System / Connection Method  

Provide exact attachment method:

- Motherboard model + revision  
- HBA or RAID controller model + firmware version  
- USB-to-SATA/NVMe adapter model (if used)  
- Product link (optional)  

### Operating System  

- OS name + version  
- Kernel/build version  
- CPU architecture (x86_64 / ARM64)  

---

## 4. Mandatory Diagnostic Data (openSeaChest)

Run:

```
openSeaChest_SMART --scan
```

Identify the correct device handle, then run the following using:

```
-d <handle>
```

### Required for ALL drives (SATA, SAS, NVMe)

```
openSeaChest_SMART -d <handle> -i
openSeaChest_SMART -d <handle> --llInfo
openSeaChest_SMART -d <handle> --smartCheck
openSeaChest_SMART -d <handle> --deviceStatistics
openSeaChest_SMART -d <handle> --shortDST
openSeaChest_SMART -d <handle> --showDSTLog
```

### Required for SATA

```
openSeaChest_SMART -d <handle> --smartAttributes raw
```

### Required for NVMe

```
openSeaChest_SMART -d <handle> --showNVMeHealth
```

---

## 4A. IDD + FARM (Strongly Recommended When Supported)

If supported by the drive (capabilities visible in `-i` output), run IDD before collecting FARM.  
This updates many diagnostic fields and provides the most current snapshot of device state.

```
openSeaChest_SMART -d <handle> --idd short --poll
openSeaChest_SMART -d <handle> --showFARM
```

Including this data significantly improves firmware root-cause analysis.

---

## 4B. Optional: Raw FARM and Telemetry Binaries

If you are willing and allowed to share unparsed diagnostic binaries through Seagate’s official disclosure process, these may accelerate investigation.

If you cannot attach binaries publicly due to policy or internal restrictions, indicate that you can provide them through Seagate’s official intake channel.

```
openSeaChest_Logs -d <handle> --farmCombined
openSeaChest_Logs -d <handle> --getTelemetry current --telemetryDataArea 3
```

These logs are intended to contain device telemetry information rather than user data.  
Only provide them if permitted by your organization.

---

## 4C. Additional Required Data for SATA/NVMe Behind HBAs, RAID Controllers, or USB Adapters

If the drive is **SATA or NVMe** and connected through:

- an HBA  
- a RAID controller  
- a USB bridge/adapter  
- any controller that may translate drive protocol behavior  

…the controller may alter or reinterpret native drive responses.

To distinguish **drive-reported** behavior from **controller-interpreted** behavior, run relevant commands using both default and forced protocol modes.

### General comparison pattern

```
openSeaChest_<tool> -d <handle> <feature>
openSeaChest_<tool> -d <handle> <feature> --forceSCSI
openSeaChest_<tool> -d <handle> <feature> --forceATA
```

### Example

```
openSeaChest_SMART -d <handle> --deviceStatistics
openSeaChest_SMART -d <handle> --deviceStatistics --forceSCSI
openSeaChest_SMART -d <handle> --deviceStatistics --forceATA
```

**This requirement does NOT apply to SAS drives**, since SAS exposes native device behavior without protocol translation.

Include all output from these comparisons.

---

## 5. Expected Behavior  

Describe what the drive or command sequence *should* have done.

---

## 6. Actual Behavior  

Describe what actually occurred:

- Errors  
- Timeouts  
- Device drops / enumeration failures  
- Drive becomes unresponsive / bricked  
- Incorrect power/EPC/standby behavior  
- SMART or log-page anomalies  
- Error behavior under load  
- Differences after power cycles  
- Any change after firmware updates  

---

## 7. Firmware Update Status  

- [ ] I updated to the latest firmware from Seagate.com and the issue **still occurs**  
- [ ] I checked Seagate.com but **no update was available**  
- [ ] The firmware update attempt **failed**  
- [ ] I have **not checked** for firmware updates yet  

If updated, describe behavior changes:

```
<Before/after firmware update observations>
```

---

## 8. Confirmation & Evidence Summary  

- [ ] All required openSeaChest outputs are included  
- [ ] I included logs/output from any other software that reproduced the issue  
- [ ] Hardware/HBA/adapter connection details are complete  
- [ ] Logs are full and unmodified  
- [ ] IDD and FARM output are included when supported  
- [ ] Raw telemetry binaries are available (if willing to share through official channel)  
- [ ] The issue appears to originate from drive firmware (based on observed data)  

---

## 9. Additional Evidence  

Attach anything else relevant:

- SMART logs  
- NVMe/SATA log pages  
- FARM output  
- Raw FARM/Telemetry binaries (if permitted)  
- Kernel/system messages  
- Screenshots  
- Diagnostic captures  
- Power-cycle notes  
- Timing or workload details  

---

# Optional: AI Assistant Collection Prompt

Use this with any AI assistant to gather required data:

```
I need to collect data for a Seagate drive firmware-defect report 
before submitting it through the Responsible Vulnerability Disclosure process.

Guide me step-by-step and validate completeness.

1) Run openSeaChest_SMART --scan and help me select the correct -d <handle>
2) Collect full raw outputs for:
   - openSeaChest_SMART -d <handle> -i
   - openSeaChest_SMART -d <handle> --llInfo
   - openSeaChest_SMART -d <handle> --smartCheck
   - openSeaChest_SMART -d <handle> --deviceStatistics
   - openSeaChest_SMART -d <handle> --shortDST
   - openSeaChest_SMART -d <handle> --showDSTLog
3) If SATA:
   - openSeaChest_SMART -d <handle> --smartAttributes raw
4) If NVMe:
   - openSeaChest_SMART -d <handle> --showNVMeHealth
5) If supported:
   - openSeaChest_SMART -d <handle> --idd short --poll
   - openSeaChest_SMART -d <handle> --showFARM
6) If SATA/NVMe behind HBA/RAID/USB:
   - collect controller vs drive comparisons using --forceSCSI and --forceATA

Also collect:
- Reproduction steps
- Expected vs actual behavior
- Firmware update status
- Additional logs or binaries (if permitted)

Finally, generate a complete Markdown report using this template.
```
