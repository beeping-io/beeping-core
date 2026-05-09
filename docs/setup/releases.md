# 🚀 Release pipeline — setup & operation

This document covers the GitHub Actions release pipeline for
`beeping-core`: how it is wired, the secrets/identities it depends
on, and how to recover when something breaks.

## TL;DR

```
develop  →  PR (merge commit)  →  main
                                   │
                                   ▼
                            release-please.yml
                            (uses GitHub App token)
                                   │
                                   ▼
                          PR "chore(main): release X.Y.Z"
                                   │
                                   ▼ merge
                              tag vX.Y.Z pushed
                                   │
                                   ▼ (fires automatically because
                                   │  app-token is NOT GITHUB_TOKEN)
                                   ▼
                              release.yml
                          builds 7-platform matrix
                          + cosign + SBOM + SLSA L3
                          → publishes GH Release
```

## Identities

| Workflow | Token used | Why |
|---|---|---|
| `release-please.yml` | `actions/create-github-app-token@v1` issuing a token for the **`beeping-io-release-please`** GitHub App | Tags pushed by `GITHUB_TOKEN` do not trigger downstream workflows. The app token has its own identity, so the `vX.Y.Z` tag push fires `release.yml`. |
| `release.yml` | `GITHUB_TOKEN` (default) + cosign keyless via OIDC | Just publishes assets to the existing tag/release. No further chaining required. |

## Secrets configured on the repo

Settings → Secrets and variables → Actions:

- **`RELEASE_PLEASE_APP_ID`** — numeric App ID of the
  `beeping-io-release-please` GitHub App (find it at
  `https://github.com/organizations/beeping-io/settings/apps/beeping-io-release-please`,
  "About" section, "App ID: ..." field).
- **`RELEASE_PLEASE_APP_PRIVATE_KEY`** — full contents of the `.pem`
  file generated for the app (including the `-----BEGIN/END RSA PRIVATE KEY-----`
  delimiter lines).

## GitHub App permissions

The `beeping-io-release-please` app must have these **repository**
permissions on `beeping-io/beeping-core` (and any other repo where it
is installed):

| Permission | Access |
|---|---|
| Contents | Read and write |
| Issues | Read and write |
| Pull requests | Read and write |
| Metadata | Read (mandatory by GitHub) |

Webhook is **not** enabled for this app — the workflow polls
release-please-action; the app does not need to receive events.

## How to set up from scratch (replicating the pattern in another repo)

1. **Use the same `beeping-io-release-please` app** (org-level, install on the new repo).
   - Go to `https://github.com/organizations/beeping-io/settings/apps/beeping-io-release-please/installations`
   - Add the new repo via "Configure"
   - That's it — no need to create a new app per repo
2. **Add the two secrets** to the new repo
   (`Settings → Secrets and variables → Actions`):
   - `RELEASE_PLEASE_APP_ID`
   - `RELEASE_PLEASE_APP_PRIVATE_KEY`
3. **Copy `release-please.yml`** from `beeping-core` and adapt:
   - Keep the `target-branch: main` if the repo uses a `develop`-as-default
     branching model. Otherwise drop the input.
   - Keep the `actions/create-github-app-token@v1` step verbatim.

## Troubleshooting

### Symptom: release-please workflow fails with `Bad credentials` / `401`

- Check that both secrets are present
  (`gh secret list --repo beeping-io/beeping-core`).
- Check that the app is **installed** on the repo (Org settings → GitHub
  Apps → `beeping-io-release-please` → "Configure" → repo list).
- If the private key was rotated in the GitHub UI, the secret in the
  repo is now stale. Regenerate the key and update
  `RELEASE_PLEASE_APP_PRIVATE_KEY`.

### Symptom: release.yml does NOT trigger after release-please merges its release PR

This means the tag was pushed by `GITHUB_TOKEN` instead of the app
token, so the workflow chain was suppressed.

1. Check `release-please.yml` actually has the
   `actions/create-github-app-token@v1` step (not just `GITHUB_TOKEN`).
2. Check the run logs of `release-please.yml` for `releaseCreated` /
   `tagName` outputs and confirm the action is using the app token.
3. **Manual recovery:**
   ```bash
   git fetch --tags origin
   git push origin --delete vX.Y.Z   # remove the tag pushed by GITHUB_TOKEN
   git push origin vX.Y.Z            # re-push from your local; fires release.yml
   ```
4. After recovery, fix the workflow so it does not happen again.

### Symptom: release-please targets the wrong branch

`target-branch: main` must be present in `release-please.yml`. Without
it, release-please picks up the repo's default branch (which in this
project is `develop`) and opens release PRs against the wrong base.

### Symptom: release-please opens a PR with the wrong starting version

`.release-please-manifest.json` is the source of truth for the current
version, NOT the git tags. If somebody pushes tags manually without
updating the manifest, release-please will compute the next bump from
the manifest (e.g. `0.1.0 + feat = 0.2.0`) instead of the latest tag
(`v0.6.0 + feat = v0.7.0`).

To recover: edit `.release-please-manifest.json` to match the latest
real tag, commit + PR through develop → main. The next release-please
run will compute correctly.

## History

| Date | Change | Reason |
|---|---|---|
| 2026-05-08 | Switched release-please from `GITHUB_TOKEN` to GitHub App token (BEE-2224) | v0.7.0 cut required a manual tag re-push because release.yml did not fire. App token is not subject to the recursive-workflow protection. |
| 2026-05-08 | Pinned `target-branch: main` + synced manifest to `0.6.0` (BEE-2224 prerequisite, fix in PR #28) | Default-branch detection picked `develop`; manifest had drifted to `0.1.0` from manual tag pushes. |
