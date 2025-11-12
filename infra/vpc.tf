module "vpc" {
  source  = "terraform-aws-modules/vpc/aws"
  version = "~> 6.0"

  name = var.name
  cidr = "10.0.0.0/16"

  azs             = ["us-east-1a"]
  private_subnets = ["10.0.1.0/24"]
  public_subnets  = ["10.0.101.0/24"]


  enable_nat_gateway = true
  single_nat_gateway = true
  #one_nat_gateway_per_az = false
}

resource "aws_security_group" "dev" {
  name_prefix = var.name
  vpc_id      = module.vpc.vpc_id

  # HTTPS for SSM + package updates.
  egress {
    from_port   = 443
    to_port     = 443
    protocol    = "tcp"
    cidr_blocks = ["0.0.0.0/0"]
  }
}