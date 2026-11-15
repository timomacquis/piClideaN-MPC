# Installation and removal

## Requirements

- Akai MPC Gen1 / Force with ARMv7 VST2 plugin support and ALSA MIDI routing. MPC OS 3.10.1 provides official VST2 folder support.
- A compatible installer that handles portable plugin ZIPs and MPC plugin registration.
- Use an executable plugin location supported by the device. A `noexec` external drive cannot load a plugin.

The package contains regular files, requires no symbolic links and can be stored on exFAT. Filesystem support does not replace the executable-mount requirement.

## Install

1. Save the current project before allowing an installer to restart MPC.
2. Open the installer's local ZIP installation page.
3. Select `ClideaN-MPC-1.3.7-mpc-armv7.zip` from `Installation/`.
4. On OS 3.10.1+, select **User Memory / VST2 Plugins**. Older compatible systems use **Synths**.
5. Review the destination and complete the installation. Allow the requested MPC restart after saving.
6. Create a Plugin Track and select **πClideaN**.
7. Create or select the target synthesizer, Drum Track or drum instrument.
8. On the target track, enable `piClideaN MPC Out` as its MIDI input and receive channel 1 or all channels.
9. Start the Force transport.

The content directory is `Timo - VST - πClideaN`. On OS 3.10.1+, the standard User Memory location is:

```text
/sdcard/VST2 Plugins/Timo - VST - πClideaN/
```

The package includes `plugin.json` with `file32` for the official loader, plus metadata for older supported hosts. Its generated `INSTALL.md` is authoritative; use the same destination for installation and removal.

Do not rename the binary, content directory or skin identifier independently. They form a single plugin identity.

## Remove

1. Stop playback and save any project changes you want to keep.
2. Remove instances from projects you intend to use without this plugin, and remove their MIDI input assignments from target tracks as needed.
3. In the compatible installer's installed-plugin management page, select **πClideaN** and choose its removal action.
4. Review the selected plugin and location. Complete removal and allow the requested MPC restart after saving.

Use the installer's removal workflow so the plugin files and its registration are handled together. Deleting a folder alone leaves its registration behind. Project files and samples are separate from the plugin package.

## Routing checklist

- πClideaN produces MIDI, not audio: its own audio output is silent.
- Enable its output on the intended target track, using the same MIDI channel.
- Match the MIDI note in CONFIG to the target pad or sound.
- Disable unwanted parallel MIDI inputs to avoid duplicate hits.
- For multiple instances, choose the corresponding numbered output port.
- In HOLD, the target must respond to Note Off; One Shot playback may ignore it.

For **Gen2 / ARM64**, use the separate [Gen2 installation guide](INSTALLATION-GEN2.md) and AArch64 package. Keep a copy of projects before updating: 1.3.0 cannot load the new saved GATE locks.
