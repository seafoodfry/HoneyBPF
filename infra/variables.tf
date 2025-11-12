variable "name" {
  type    = string
  default = "honeybpf" # NOTE: existing key pair.
}

variable "ec2_key_pair" {
  type    = string
  default = "numerical-recipes" # NOTE: existing key pair.
}