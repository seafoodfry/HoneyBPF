output "ami_id" {
  value = data.aws_ami.ami.id
}

output "instance_id" {
  value = aws_instance.ec2.id
}