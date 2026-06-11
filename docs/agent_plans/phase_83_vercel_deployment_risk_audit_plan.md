# Phase 83 - Vercel Deployment Risk Audit

## Goal

Audit the repository for risks if it is deployed or connected to Vercel.

## Checklist

- [x] Review repository agent rules and Vercel deployment guidance.
- [x] Inspect repository shape for Vercel, Node, framework, and CI configuration.
- [x] Inspect CMake, dependency, generated-project, and documentation signals that affect Vercel deployability.
- [x] Summarize deployment risks with severity and concrete mitigation options.
- [x] Record validation status and final result summary.

## Validation Status

- Source/config inspection only. No build, test, app launch, generated app launch, repository build script, generated build script, terminal build command, PowerShell build command, or `cmd.exe` build command was run.
- Vercel-specific local config/secret sweep found no committed `vercel.json`, `.vercel`, package lock/manifest, `.env`, key, token, or obvious secret file in the searched repository surface.

## Remaining TODOs

- Decide whether Vercel should be used only for a future docs/marketing/web companion app, or whether VisiForm needs a separate web/WASM architecture before Vercel is a deployment target.
- If a Vercel-hosted companion app is desired, add it in an isolated subdirectory and configure Vercel's root directory to that subproject.
- Pin `VISIFORM_VISAGE_GIT_TAG` to a tested commit before any remote CI/build pipeline depends on FetchContent.

## Final Result Summary

The repository is not ready for direct Vercel deployment as the primary application. It is a native C++20 / CMake / Visage desktop application that builds an executable target named `VisiForm`, documents Windows 10/11 and Visual Studio 2022 as the primary supported path, and marks macOS/Linux support as experimental. No Vercel configuration, Node package manifest, frontend framework config, or deployment workflow was found.

Primary risks:

- Critical: direct Vercel deployment of the main app is the wrong target shape. Vercel expects a web/static/functions deployment surface, while this repository produces a native GUI executable.
- High: Vercel's Linux remote build environment would not match the documented primary Windows/MSVC/static-runtime workflow, and the non-Windows paths are explicitly not production-validated.
- High: the Visage fallback dependency currently defaults to `VISIFORM_VISAGE_GIT_TAG=main`, which makes remote builds non-reproducible.
- Medium: there is no Vercel root isolation or framework configuration, so connecting the repo as-is would likely fail auto-detection or build the wrong thing.
- Medium: tests and CI are not wired into a Vercel-compatible pipeline in this repo; the Catch2 test target exists separately but is not integrated into the root CMake file.
- Low: no Vercel secrets or local project metadata appear to be committed.
