# Infra

---
# TL;DR

Make sure your `~/.aws/config` has a profile like so:
```toml
[profile terraform]
credential_process = aws configure export-credentials --profile default --format process
```

then you can:
```bash
aws login

export AWS_PROFILE=terraform

terraform init

aws logout
```

---
## AWS

As per [AWS: Login credentials provider](https://docs.aws.amazon.com/sdkref/latest/guide/feature-login-credentials.html),
the SDK for Go V2 (1.x) does not yet support the login credentials provider.

Because of that, Terraform will fail after running `aws login` successfully.

To resolve this, one could create a script such as
```bash
#!/bin/bash
# source this: source ./aws-creds.sh
set -euo pipefail

PROFILE="${AWS_PROFILE:-default}"

echo "Loading AWS credentials from profile: ${PROFILE}" >&2
eval "$(aws configure export-credentials --profile ${PROFILE} --format env)"

echo "✓ Credentials loaded for:" >&2
aws sts get-caller-identity --output table >&2
```

But we could also add the following profile to `~/.aws/config`
```toml
[profile terraform]
credential_process = aws configure export-credentials --profile default --format process
```

Once we got the above, the following should work
```bash
export AWS_PROFILE=terraform
terraform init
...
```