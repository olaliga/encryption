class Bottleneck(Module):
  __parameters__ = []
  __buffers__ = []
  training : bool
  _is_full_backward_hook : Optional[bool]
  conv1 : __torch__.torch.nn.modules.conv.___torch_mangle_62.Conv2d
  bn1 : __torch__.torch.nn.modules.batchnorm.___torch_mangle_63.BatchNorm2d
  conv2 : __torch__.torch.nn.modules.conv.___torch_mangle_64.Conv2d
  bn2 : __torch__.torch.nn.modules.batchnorm.___torch_mangle_65.BatchNorm2d
  conv3 : __torch__.torch.nn.modules.conv.___torch_mangle_66.Conv2d
  bn3 : __torch__.torch.nn.modules.batchnorm.___torch_mangle_67.BatchNorm2d
  relu : __torch__.torch.nn.modules.activation.___torch_mangle_68.ReLU
  downsample : __torch__.torch.nn.modules.container.___torch_mangle_71.Sequential
  def forward(self: __torch__.torchvision.models.resnet.___torch_mangle_72.Bottleneck,
    argument_1: Tensor) -> Tensor:
    downsample = self.downsample
    bn3 = self.bn3
    conv3 = self.conv3
    bn2 = self.bn2
    conv2 = self.conv2
    relu = self.relu
    bn1 = self.bn1
    conv1 = self.conv1
    _0 = (bn1).forward((conv1).forward(argument_1, ), )
    _1 = (conv2).forward((relu).forward(_0, ), )
    _2 = (relu).forward1((bn2).forward(_1, ), )
    _3 = (bn3).forward((conv3).forward(_2, ), )
    input = torch.add_(_3, (downsample).forward(argument_1, ))
    return (relu).forward2(input, )
