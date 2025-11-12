terraform {
  backend "s3" {
    bucket = "seafoodfry-tf-backend"
    key    = "honeybpf/infra/terraform.tfstate"
    region = "us-east-2"
  }

  required_providers {
    aws = {
      source  = "hashicorp/aws"
      version = "~> 6.12"
    }
  }

  required_version = ">= 1.13"
}

provider "aws" {
  alias  = "us_east_2"
  region = "us-east-2"
}

provider "aws" {
  #alias = "us_east_1"
  region = "us-east-1"
}