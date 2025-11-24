# Infra


Make sure your `~/.aws/config` has a profile like so:
```toml
[profile terraform]
credential_process = aws configure export-credentials --profile default --format process
```

then you can:
```bash
aws login

export AWS_PROFILE=terraform

# Spin up the dev ec2.
make

# Get the env var to configure for the next step.
make instance_id
# i.e., export INSTANCE_ID=i-...

# Start the instance manager ssh session.
make ssh
```