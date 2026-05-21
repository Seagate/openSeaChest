---
description: 'Source Control Management best practices for the openSeaChest GitHub repository — branch protection, code review, secrets, and organization security settings'
applyTo: '.github/**'
---

# SCM (Source Control Management) Best Practices — openSeaChest

Source: [OpenSSF SCM Best Practices](https://best.openssf.org/SCM-BestPractices/)

---

## Repository Settings

### Branch Protection (`develop` and `main`)

The following protections must be active on the default branch (`develop`) and any release branches:

- **Require pull request reviews before merging** — minimum 1 reviewer; 2 is recommended for security-sensitive changes.
- **Dismiss stale pull request approvals when new commits are pushed** — prevents approving a clean diff then sneaking in changes.
- **Require status checks to pass before merging** — all CI jobs (Windows, Linux, FreeBSD) must pass.
- **Require branches to be up to date before merging** — prevents merging stale branches that haven't seen recent CI results.
- **Require conversation resolution before merging** — all review comments must be addressed.
- **Do not allow force pushes** — preserves commit history; required for audit and reproducible-build verification.
- **Do not allow deletions** — the default branch cannot be deleted.
- **Require signed commits** (recommended) — sign commits with GPG or SSH keys to verify author identity.

### Automated Security Features

Ensure these are enabled in the repository's **Security** settings:

- **Dependabot alerts** — notifies on known vulnerabilities in dependencies and submodules.
- **Dependabot security updates** — automatically opens PRs to fix vulnerable dependency versions.
- **Secret scanning** — detects leaked credentials in pushes and repository content.
- **Push protection** — blocks pushes containing detected secrets at push time (not just after).
- **Code scanning / CodeQL** — static analysis via GitHub Actions (see `.github/workflows/`).

---

## GitHub Actions Workflow Security

### Permissions

- Workflows must declare the minimum required permissions. Default to `permissions: read-all` at the workflow level and override narrowly for specific jobs:

```yaml
permissions:
  contents: read       # most jobs only need to read the repo
  pull-requests: write # only for jobs that post PR comments
  security-events: write # only for CodeQL SARIF upload
```

- **Workflows must not approve pull requests** — this prevents a workflow from self-approving its own changes.
- **GITHUB_TOKEN should be read-only by default.** Escalate to write only in specific jobs that require it (e.g., release publishing).

### Action Pinning

- Pin third-party GitHub Actions to a specific commit SHA, not a mutable tag:

```yaml
# WRONG — tag can be moved
- uses: actions/checkout@v4

# CORRECT — pinned to a specific commit
- uses: actions/checkout@11bd71901bbe5b1630ceea73d27597364c9af683 # v4.2.2
```

- Use only **verified** or **trusted** Actions. Prefer Actions from `actions/`, `github/`, or well-known publishers.
- Review the source code and permissions of any third-party Action before adding it.

### Secret Handling in Workflows

- Access secrets via `${{ secrets.SECRET_NAME }}` — never echo them, log them, or write them to files.
- Do not pass secrets as plain environment variables to scripts that might log their environment.
- Do not use `pull_request_target` with code from forks unless you understand the security model — it grants fork PRs access to secrets.

---

## Organization Security Settings

- **Enforce MFA/2FA** for all organization members with write access.
- **Single Sign-On (SSO)** should be required where the organization uses an identity provider.
- **Default member permissions** should be set to the minimum required (typically "Read").
- **Outside collaborators** should be reviewed periodically and removed when no longer needed.
- Keep the number of **repository owners and organization admins** small — ideally fewer than 3 active admins.

---

## Commit Hygiene

- Commit messages must follow the project's `commit_template.txt` format (summary line + body + trailer).
- Each commit should represent a single logical change. Avoid "WIP" commits on shared branches.
- Do not commit generated files (build artefacts, autoconf output, compiled objects).
- Do not commit credentials, private keys, or any file containing secrets — ever. The `tool-guardian` hook and GitHub push protection are last-resort defenses, not the primary safeguard.

## Pull Request Workflow

- Open PRs against `develop`, not directly against `main`/release branches.
- Keep PRs focused and small. Large PRs are harder to review securely.
- Link PRs to the relevant issue or task.
- Security-sensitive changes (authentication, privilege escalation, command execution, crypto) warrant a dedicated security-focused review pass before merge.

---

## Access Review

- Review repository collaborators, team memberships, and organization members **at least quarterly**.
- Revoke access promptly when a contributor leaves the project or organization.
- Audit the GitHub Actions secrets list and remove unused secrets.
- Review Dependabot auto-merge settings and ensure security updates are still being applied.
