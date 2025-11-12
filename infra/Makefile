.PHONY: all
all:
	terraform fmt
	./run-cmd-in-shell.sh terraform init
	./run-cmd-in-shell.sh terraform plan -out a.plan
	echo "./run-cmd-in-shell.sh terraform apply a.plan"

.PHONY: instance_id
instance_id:
	echo "export INSTANCE_ID=$$(./run-cmd-in-shell.sh terraform output -raw instance_id)"

.PHONY: ssh
ssh:
	./run-cmd-in-shell.sh aws ssm start-session --region us-east-1 \
    --target ${INSTANCE_ID} \
    --document-name AWS-StartSSHSession \
    --parameters portNumber=22

.PHONY: sync
sync:
	rsync -rvzP ../tutorials ${INSTANCE_ID}:/home/ec2-user

.PHONY: destroy
destroy:
	./run-cmd-in-shell.sh terraform destroy

.PHONY: clean
clean:
	rm -f a.plan
	rm -f terraform.tfstate*
	rm -rf .terraform*
