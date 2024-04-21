class ResNet(Module):
  __parameters__ = []
  __buffers__ = []
  training : bool
  _is_full_backward_hook : Optional[bool]
  conv1 : __torch__.torch.nn.modules.conv.Conv2d
  bn1 : __torch__.torch.nn.modules.batchnorm.BatchNorm2d
  relu : __torch__.torch.nn.modules.activation.ReLU
  maxpool : __torch__.torch.nn.modules.pooling.MaxPool2d
  layer1 : __torch__.torch.nn.modules.container.___torch_mangle_25.Sequential
  layer2 : __torch__.torch.nn.modules.container.___torch_mangle_61.Sequential
  layer3 : __torch__.torch.nn.modules.container.___torch_mangle_113.Sequential
  layer4 : __torch__.torch.nn.modules.container.___torch_mangle_141.Sequential
  avgpool : __torch__.torch.nn.modules.pooling.AdaptiveAvgPool2d
  fc : __torch__.torch.nn.modules.linear.Linear
  def forward(self: __torch__.torchvision.models.resnet.ResNet,
    x: Tensor) -> Tensor:
    fc = self.fc
    avgpool = self.avgpool
    layer4 = self.layer4
    layer3 = self.layer3
    layer2 = self.layer2
    layer1 = self.layer1
    maxpool = self.maxpool
    relu = self.relu
    bn1 = self.bn1
    conv1 = self.conv1
    _0 = (bn1).forward((conv1).forward(x, ), )
    _1 = (maxpool).forward((relu).forward(_0, ), )
    _2 = (layer2).forward((layer1).forward(_1, ), )
    _3 = (layer4).forward((layer3).forward(_2, ), )
    input = torch.flatten((avgpool).forward(_3, ), 1)
    return (fc).forward(input, )
class Bottleneck(Module):
  __parameters__ = []
  __buffers__ = []
  training : bool
  _is_full_backward_hook : Optional[bool]
  conv1 : __torch__.torch.nn.modules.conv.___torch_mangle_0.Conv2d
  bn1 : __torch__.torch.nn.modules.batchnorm.___torch_mangle_1.BatchNorm2d
  conv2 : __torch__.torch.nn.modules.conv.___torch_mangle_2.Conv2d
  bn2 : __torch__.torch.nn.modules.batchnorm.___torch_mangle_3.BatchNorm2d
  conv3 : __torch__.torch.nn.modules.conv.___torch_mangle_4.Conv2d
  bn3 : __torch__.torch.nn.modules.batchnorm.___torch_mangle_5.BatchNorm2d
  relu : __torch__.torch.nn.modules.activation.___torch_mangle_6.ReLU
  downsample : __torch__.torch.nn.modules.container.Sequential
  def forward(self: __torch__.torchvision.models.resnet.Bottleneck,
    argument_1: Tensor) -> Tensor:
    downsample = self.downsample
    bn3 = self.bn3
    conv3 = self.conv3
    bn2 = self.bn2
    conv2 = self.conv2
    relu = self.relu
    bn1 = self.bn1
    conv1 = self.conv1
    _4 = (bn1).forward((conv1).forward(argument_1, ), )
    _5 = (conv2).forward((relu).forward(_4, ), )
    _6 = (relu).forward1((bn2).forward(_5, ), )
    _7 = (bn3).forward((conv3).forward(_6, ), )
    input = torch.add_(_7, (downsample).forward(argument_1, ))
    return (relu).forward2(input, )
