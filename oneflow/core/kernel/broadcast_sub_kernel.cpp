#include "oneflow/core/kernel/broadcast_sub_kernel.h"
#include "oneflow/core/ndarray/binary_func.h"
#include "oneflow/core/ndarray/unary_func.h"
#include "oneflow/core/ndarray/xpu_ndarray_util.h"

namespace oneflow {

template<DeviceType device_type, typename T>
void BroadcastSubKernel<device_type, T>::ForwardDataContent(
    const KernelCtx& kernel_ctx, std::function<Blob*(const std::string&)> BnInOp2Blob) const {
  BroadcastBinaryKernelUtil<device_type, T, BinaryFuncSub>::Forward(kernel_ctx, BnInOp2Blob);
}

template<DeviceType device_type, typename T>
void BroadcastSubKernel<device_type, T>::BackwardDataContent(
    const KernelCtx& kernel_ctx, std::function<Blob*(const std::string&)> BnInOp2Blob) const {
  const Blob* out_diff_blob = BnInOp2Blob("out_diff");
  Blob* bw_buf_blob = BnInOp2Blob("bw_buf");
  Blob* a_diff_blob = BnInOp2Blob("a_diff");
  Blob* b_diff_blob = BnInOp2Blob("b_diff");
  size_t num_axes = out_diff_blob->shape().NumAxes();
  if (a_diff_blob) {
    XpuNdArrayUtil<device_type, T>::SwitchReduce(
        SwitchCase(num_axes), kernel_ctx.device_ctx, XpuVarNdarray<T>(a_diff_blob, num_axes),
        XpuVarNdarray<const T>(out_diff_blob, num_axes), XpuVarNdarray<T>(bw_buf_blob, num_axes));
  }
  if (b_diff_blob) {
    XpuNdArrayUtil<device_type, T>::SwitchReduce(
        SwitchCase(num_axes), kernel_ctx.device_ctx, XpuVarNdarray<T>(b_diff_blob, num_axes),
        XpuVarNdarray<const T>(out_diff_blob, num_axes), XpuVarNdarray<T>(bw_buf_blob, num_axes));
    XpuNdArrayUtil<device_type, T>::template ImplaceApplyUnary<UnaryFuncMinus>(
        kernel_ctx.device_ctx, XpuVarNdarray<T>(b_diff_blob, num_axes));
  }
}

ADD_DEFAULT_KERNEL_CREATOR(OperatorConf::kBroadcastSubConf, BroadcastSubKernel,
                           FLOATING_DATA_TYPE_SEQ);
}  // namespace oneflow
