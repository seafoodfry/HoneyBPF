#!/bin/bash
set -x
set -e

systemctl enable amazon-ssm-agent
systemctl restart amazon-ssm-agent

sudo yum update -y
sudo yum install rsync tmux bash-completion -y
sudo yum groupinstall "Development Tools" -y