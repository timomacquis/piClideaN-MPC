# Repository rename — 10 November 2026

Canonical public URL: https://github.com/timomacquis/piClideaN-MPC

The former piClideaN16-MPC repository was renamed in place. Commit history, tags, issues and release assets remain attached to the same repository. Local public remotes and visible README links use the new URL. No plugin files or release assets were rebuilt or replaced.

## Catalog compatibility

The standard catalog checker requires `manifest.source_repo` to equal the registry `repo` exactly. Existing releases use `timomacquis/piClideaN16-MPC`. Keep that historical alias in the registry and in package metadata for now; GitHub redirects it to the renamed repository. Current screenshot links may use the canonical new URL, pinned to a commit.

A future metadata migration must update the registry and package manifests together. Do not change only the registry repo field, which would reject the existing ZIPs. The catalog ID `piclidean-mpc`, VST UID, engine filename, folders, parameter indices and project state are unchanged.

Do not reuse the old repository name for a different repository: GitHub's redirect would be lost.
