data "aws_iam_policy_document" "trust_relationship" {
  statement {
    effect = "Allow"
    principals {
      type        = "Service"
      identifiers = ["ec2.amazonaws.com"]
    }
    actions = ["sts:AssumeRole"]
  }
}

resource "aws_iam_role" "ssm" {
  name               = var.name
  path               = "/"
  assume_role_policy = data.aws_iam_policy_document.trust_relationship.json
}

resource "aws_iam_instance_profile" "ssm" {
  name = var.name
  role = aws_iam_role.ssm.name
}

resource "aws_iam_role_policy_attachment" "ssm" {
  role       = aws_iam_role.ssm.name
  policy_arn = "arn:aws:iam::aws:policy/AmazonSSMManagedInstanceCore"
}