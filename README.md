# Winlogon Configuration Tool (winlogoncfg)

A native Windows GUI for viewing and editing selected Winlogon registry settings on the local computer or a remote computer. It covers logon defaults, automatic logon, session startup, the logon dialog and legacy Windows File Protection settings.

This is a historical tool with documentation from 2005–2007, written around Windows NT, 2000, XP and Server 2003. The source and makefile also contain ARM64 build paths, but this does not establish that every setting works on current Windows versions. The original help describes the behavior of those older systems.

## Settings

The application reads and writes values under:

```text
HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows NT\CurrentVersion\Winlogon
```

| Area | Registry values exposed by the UI |
| --- | --- |
| Logon defaults | `DefaultUserName`, `DefaultPassword`, `DefaultDomainName` |
| Automatic logon | `AutoAdminLogon`, `IgnoreShiftOverride`, `ForceAutoLogon` |
| Session startup and behavior | `Userinit`, `Shell`, `RunLogonScriptSync`, `AllowMultipleTSSessions`, `DisableLockWorkstation`, `AutoRestartShell` |
| Logon dialog | `ShutdownWithoutLogon`, `ShowLogonOptions`, `DontDisplayLastUserName`, `DisableCAD` |
| Legacy Windows File Protection | `SFCDisable`, `SfcQuota` |

The Windows File Protection controls are enabled only when the corresponding DWORD values can be read. The table describes the values this program edits, not a guarantee that the target Windows version honors them. The application does not discover newer policy locations or validate the resulting logon behavior.

## Usage

1. Launch `winlogoncfg.exe`. It opens the local computer's settings.
2. To select another computer, use **File → Open remote computer...** and enter its name. **File → Open local computer** returns to the local registry.
3. Review the target shown in the title bar and edit the settings.
4. Choose **File → Save changes** to write them. Closing the application or switching computers prompts about unsaved edits.

Remote access uses `RegConnectRegistry` and the caller's existing access rights; there is no separate credential dialog. Saving requires write access to the selected registry key.

**The password is stored as plain text in the `DefaultPassword` registry value.** The **Hide password characters while typing** option only masks the field on screen. Saving a nonempty password writes it even if automatic logon is disabled.

Save processes the whole form, including values you have not just edited. Empty username, password, domain, user-init and shell fields delete the corresponding values. Several checkboxes have an indeterminate state that can delete registry values; it is not a general “leave unchanged” mode.

Back up the relevant registry settings before saving, especially before changing `Userinit` or `Shell`. Writes are performed individually, with no transaction or automatic rollback; a save error can leave some settings changed. A successful save confirms the registry operations, not that a future logon will work as intended.

## Help and repository contents

| File | Purpose |
| --- | --- |
| [winlogoncfg.cpp](winlogoncfg.cpp) | GUI logic and registry reads/writes |
| [winlogoncfg.rc](winlogoncfg.rc), [winlogoncfg.rc.h](winlogoncfg.rc.h) | Dialogs, menus, version information and resource identifiers |
| [Makefile](Makefile) | NMAKE executable build |
| [winlogoncfg.hlp](winlogoncfg.hlp), [winlogoncfg.cnt](winlogoncfg.cnt) | Original compiled WinHelp documentation and contents |
| [winlogoncfg.rtf](winlogoncfg.rtf), [winlogoncfg.hpj](winlogoncfg.hpj) | Help source and Help Workshop project |

**F1** and the **Help** menu call the legacy WinHelp API and require a working WinHelp viewer. The RTF source can also be read separately with an RTF-capable application. Keep its original platform-specific guidance in historical context.

## Building

The makefile uses Microsoft **NMAKE**, Visual C++ (`cl` and `link`) and the Windows resource compiler (`rc`). There is no Visual Studio solution or project file.

The build expects shared LTR Data headers from [LTRData/include](https://github.com/LTRData/include), including `winstrct.h` and `winstrct.hpp`, plus the associated `winstrct.lib` and `winstrcp.lib` libraries. The source also links `ntdllp.lib`, and the makefile lists `..\lib\minwcrt.lib` as a prerequisite. These dependencies are not bundled here.

Prepare the compiler's include/library paths and the output directory before running `nmake`. The makefile takes `CPU` from `_BUILDARCH` when available and otherwise defaults to `i386`; objects and the executable go under that directory. It has special cases for `i386` and `ARM64`, and selects debug settings when `DEBUG` is nonempty.

This is a legacy build recipe with custom dependencies and older compiler/linker options that may need adaptation to the installed toolchain. The `install` target uses a maintainer-specific `P:\utils` destination. Rebuilding the WinHelp file is separate from the executable build and uses the supplied Help Workshop project.

## License

[MIT License](LICENSE), by Olof Lagerkvist. The standalone license reproduces the notice already present in the original help source; the original help files are retained.
