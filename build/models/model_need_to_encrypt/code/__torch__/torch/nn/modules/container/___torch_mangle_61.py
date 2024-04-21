class Sequential(Module):
  __parameters__ = []
  __buffers__ = []
  training : bool
  _is_full_backward_hook : Optional[bool]
  __annotations__["0"] = __torch__.torchvision.models.resnet.___torch_mangle_36.Bottleneck
  __annotations__["1"] = __torch__.torchvision.models.resnet.___torch_mangle_44.Bottleneck
  __annotations__["2"] = __torch__.torchvision.models.resnet.___torch_mangle_52.Bottleneck
  __annotations__["3"] = __torch__.torchvision.models.resnet.___torch_mangle_60.Bottleneck
  def forward(self: __torch__.torch.nn.modules.container.___torch_mangle_61.Sequential,
    argument_1: Tensor) -> Tensor:
    _3 = getattr(self, "3")
    _2 = getattr(self, "2")
    _1 = getattr(self, "1")
    _0 = getattr(self, "0")
    _4 = (_1).forward((_0).forward(argument_1, ), )
    return (_3).forward((_2).forward(_4, ), )
