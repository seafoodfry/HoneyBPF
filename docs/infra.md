# Infra

```bash
# Spin up the dev ec2.
make

# Get the env var to configure for the next step.
make instance_id
# i.e., export INSTANCE_ID=i-...

# Start the instance manager ssh session.
make ssh
```