#.DEFAULT_GOAL := help

.PHONY: help
help:  ## Display this help message
	@awk 'BEGIN {FS = ":.*##"; printf "\nUsage:\n  make \033[36m<target>\033[0m\n"} /^[a-zA-Z_-]+:.*?##/ { printf "  \033[36m%-20s\033[0m %s\n", $$1, $$2 } /^##@/ { printf "\n\033[1m%s\033[0m\n", substr($$0, 5) } ' $(MAKEFILE_LIST)


##@ Infra Development.
.PHONY: all
all:  ## Run Terraform plan
	terraform fmt
	./run-cmd-in-shell.sh terraform init
	./run-cmd-in-shell.sh terraform plan -out a.plan
	echo "./run-cmd-in-shell.sh terraform apply a.plan"

##@ Local Development.
.PHONY: instance_id
instance_id:  ## Get EC2 instance ID
	echo "export INSTANCE_ID=$$(./run-cmd-in-shell.sh terraform output -raw instance_id)"

.PHONY: ssh
ssh:  # Create SSH tunnel with SSM
	./run-cmd-in-shell.sh aws ssm start-session --region us-east-1 \
    --target ${INSTANCE_ID} \
    --document-name AWS-StartSSHSession \
    --parameters portNumber=22

.PHONY: sync
sync:  ## Sync source code to EC2 via rsync
	rsync -rvzP ../tutorials ${INSTANCE_ID}:/home/ec2-user

##@ Infra Cleanup.
.PHONY: destroy
destroy:  ## Destroy the infras
	./run-cmd-in-shell.sh terraform destroy

.PHONY: clean
clean:  ## Delete Terraform artifacts
	rm -f a.plan
	rm -f terraform.tfstate*
	rm -rf .terraform*
