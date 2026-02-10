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

openSeaChest **cannot** accept firmware defect reports directly, and will automatically close such submissions.  
Firmware defects may still need evaluation by Seagate engineering, and in many cases they qualify as **security defects** (for example, if the drive becomes unresponsive or “bricked”).

To submit to Seagate, use:

🔗 https://www.seagate.com/legal/policies/responsible-vulnerability-disclosure-policy/

---

## 1. Purpose  
Some firmware anomalies represent security defects. Others are functional issues requiring engineering review.  
This template helps you gather all necessary data to ensure your report is complete when submitted to Seagate’s Responsible Vulnerability Disclosure process.

---

## 2. Reproduction Steps  
Provide exact, copy-pasteable steps to reliably reproduce the issue.

1.  
2.  
3.  

If intermittent, describe the frequency and conditions (thermal, load, idle time, power cycles, etc.).

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

Provide system outputs if available:
```
lspci
lsusb
openSeaChest --llInfo
```

### Operating System  
- OS name + version  
- Kernel/build version  
- CPU architecture (x86_64 / ARM64)  

---

## 4. Mandatory Diagnostic Data (openSeaChest)

The following commands **must** be executed and their full, raw output included unmodified.

### Required for ALL drives (SATA, SAS, NVMe)
```
openSeaChest --scan
openSeaChest -i
openSeaChest --llInfo
openSeaChest --smartCheck
openSeaChest --deviceStatistics
openSeaChest --shortDST
openSeaChest --showDSTLog
```

### Required for SATA
```
openSeaChest --smartAttributes raw
```

### Required for NVMe
```
openSeaChest --showNVMeHealth
```

---

## 4A. Additional Required Data for SATA/NVMe Behind HBAs, RAID Controllers, or USB Adapters

If the drive is **SATA or NVMe** and is connected through:

- an HBA  
- a RAID controller  
- a USB bridge/adapter  
- any controller that may translate drive protocol behavior  

…the controller may alter, filter, or reinterpret native drive responses.

To distinguish **drive-reported** behavior from **controller-interpreted** behavior, run each relevant openSeaChest feature in both default and forced protocol modes.

### General comparison patterns  
Use these across any feature involved in diagnosing the issue:

```
openSeaChest <feature>                     # Normal (controller-interpreted)
openSeaChest <feature> --forceSCSI         # Force SCSI translation path
openSeaChest <feature> --forceATA          # Force ATA path (if supported)
```

### Examples (NOT limited to EPC or power management)
```
openSeaChest --deviceStatistics
openSeaChest --deviceStatistics --forceSCSI
openSeaChest --deviceStatistics --forceATA

openSeaChest --showEPCSettings
openSeaChest --showEPCSettings --forceSCSI
```

### Notes  
- This helps identify whether the issue originates in **drive firmware** or **HBA/bridge firmware**.  
- **This requirement does NOT apply to SAS drives**, since SAS provides native device data without translation.  
- EPC was only an example; forced protocol modes apply across many diagnostic paths.

Include the output from:
```
<controller-reported output>
<forced SCSI output>
<forced ATA output (if available)>
```

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

Describe behavior if updated:
```
<Before/after firmware update observations>
```

---

## 8. Confirmation & Evidence Summary  

- [ ] All required openSeaChest outputs are included  
- [ ] I included logs/output from any other software that reproduced the issue  
- [ ] Hardware/HBA/adapter connection details are complete  
- [ ] Logs are full and unmodified  
- [ ] The issue appears to originate from drive firmware (based on observed data)  

---

## 9. Additional Evidence  
Attach anything else relevant:

- SMART logs  
- NVMe/SATA log pages  
- Kernel/system messages  
- Screenshots  
- Diagnostic captures  
- Power-cycle notes  
- Timing or workload details  

---

# Optional: AI Assistant Collection Prompt

Use this with any AI assistant to help gather all fields:

```
I need to collect data for a Seagate drive firmware-defect report 
before submitting it through the Responsible Vulnerability Disclosure process.

Guide me step-by-step to gather:

- Drive model, serial number, firmware version
- Host system hardware (motherboard, HBA, RAID card, USB adapter)
- OS and kernel/build versions
- All required openSeaChest commands:
  --scan
  -i
  --llInfo
  --smartCheck
  --deviceStatistics
  --shortDST
  --showDSTLog
  --smartAttributes raw (SATA only)
  --showNVMeHealth (NVMe only)

Also check whether the drive is SATA or NVMe behind an HBA/RAID/USB adapter.
If it is, help me collect:
  <feature>
  <feature> --forceSCSI
  <feature> --forceATA   (if supported)

This comparison helps distinguish controller behavior from drive firmware behavior.

Then collect:
- Reproduction steps
- Expected vs actual behavior
- Firmware update status
- Additional logs or evidence

Validate each answer for completeness
and output a final Markdown report using the provided template.
```

