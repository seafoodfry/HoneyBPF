data "aws_ami" "ami" {
  most_recent = true
  owners      = ["amazon"]

  filter {
    name   = "name"
    values = ["al2023-ami-*-arm64"] # i.e., "al2023-ami-2023.8.20250818.0-kernel-6.1-arm64"
  }
}

resource "aws_instance" "ec2" {
  instance_market_options {
    market_type = "spot"
    spot_options {
      max_price = "0.05"
    }
  }

  ami           = data.aws_ami.ami.id
  instance_type = "t4g.small"
  key_name      = var.ec2_key_pair

  subnet_id              = module.vpc.private_subnets[0]
  vpc_security_group_ids = [aws_security_group.dev.id]
  iam_instance_profile   = aws_iam_instance_profile.ssm.name

  root_block_device {
    delete_on_termination = true
    encrypted             = true
    volume_type           = "gp3"
    volume_size           = 50
  }

  metadata_options {
    http_tokens                 = "required"
    http_put_response_hop_limit = 1
  }

  user_data = file("${path.module}/setup.sh")

  tags = {
    Name = var.name
  }
}