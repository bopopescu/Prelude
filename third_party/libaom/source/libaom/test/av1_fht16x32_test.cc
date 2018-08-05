/*
 * Copyright (c) 2016, Alliance for Open Media. All rights reserved
 *
 * This source code is subject to the terms of the BSD 2 Clause License and
 * the Alliance for Open Media Patent License 1.0. If the BSD 2 Clause License
 * was not distributed with this source code in the LICENSE file, you can
 * obtain it at www.aomedia.org/license/software. If the Alliance for Open
 * Media Patent License 1.0 was not distributed with this source code in the
 * PATENTS file, you can obtain it at www.aomedia.org/license/patent.
 */

#include "third_party/googletest/src/googletest/include/gtest/gtest.h"

#include "./aom_dsp_rtcd.h"
#include "./av1_rtcd.h"

#include "aom_ports/mem.h"
#include "test/acm_random.h"
#include "test/clear_system_state.h"
#include "test/register_state_check.h"
#include "test/transform_test_base.h"
#include "test/util.h"

using libaom_test::ACMRandom;

namespace {
typedef void (*IhtFunc)(const tran_low_t *in, uint8_t *out, int stride,
                        const TxfmParam *txfm_param);
using std::tr1::tuple;
using libaom_test::FhtFunc;
typedef tuple<FhtFunc, IhtFunc, int, aom_bit_depth_t, int> Ht16x32Param;

void fht16x32_ref(const int16_t *in, tran_low_t *out, int stride,
                  TxfmParam *txfm_param) {
  av1_fht16x32_c(in, out, stride, txfm_param);
}

void iht16x32_ref(const tran_low_t *in, uint8_t *out, int stride,
                  const TxfmParam *txfm_param) {
  av1_iht16x32_512_add_c(in, out, stride, txfm_param);
}

class AV1Trans16x32HT : public libaom_test::TransformTestBase,
                        public ::testing::TestWithParam<Ht16x32Param> {
 public:
  virtual ~AV1Trans16x32HT() {}

  virtual void SetUp() {
    fwd_txfm_ = GET_PARAM(0);
    inv_txfm_ = GET_PARAM(1);
    pitch_ = 16;
    height_ = 32;
    fwd_txfm_ref = fht16x32_ref;
    inv_txfm_ref = iht16x32_ref;
    bit_depth_ = GET_PARAM(3);
    mask_ = (1 << bit_depth_) - 1;
    num_coeffs_ = GET_PARAM(4);
    txfm_param_.tx_type = GET_PARAM(2);
  }
  virtual void TearDown() { libaom_test::ClearSystemState(); }

 protected:
  void RunFwdTxfm(const int16_t *in, tran_low_t *out, int stride) {
    fwd_txfm_(in, out, stride, &txfm_param_);
  }

  void RunInvTxfm(const tran_low_t *out, uint8_t *dst, int stride) {
    inv_txfm_(out, dst, stride, &txfm_param_);
  }

  FhtFunc fwd_txfm_;
  IhtFunc inv_txfm_;
};

TEST_P(AV1Trans16x32HT, AccuracyCheck) { RunAccuracyCheck(4, 0.2); }
TEST_P(AV1Trans16x32HT, CoeffCheck) { RunCoeffCheck(); }
TEST_P(AV1Trans16x32HT, MemCheck) { RunMemCheck(); }
TEST_P(AV1Trans16x32HT, InvCoeffCheck) { RunInvCoeffCheck(); }
TEST_P(AV1Trans16x32HT, InvAccuracyCheck) { RunInvAccuracyCheck(4); }

using std::tr1::make_tuple;
const Ht16x32Param kArrayHt16x32Param_c[] = {
  make_tuple(&av1_fht16x32_c, &av1_iht16x32_512_add_c, 0, AOM_BITS_8, 512),
  make_tuple(&av1_fht16x32_c, &av1_iht16x32_512_add_c, 1, AOM_BITS_8, 512),
  make_tuple(&av1_fht16x32_c, &av1_iht16x32_512_add_c, 2, AOM_BITS_8, 512),
  make_tuple(&av1_fht16x32_c, &av1_iht16x32_512_add_c, 3, AOM_BITS_8, 512),
#if CONFIG_EXT_TX
  make_tuple(&av1_fht16x32_c, &av1_iht16x32_512_add_c, 4, AOM_BITS_8, 512),
  make_tuple(&av1_fht16x32_c, &av1_iht16x32_512_add_c, 5, AOM_BITS_8, 512),
  make_tuple(&av1_fht16x32_c, &av1_iht16x32_512_add_c, 6, AOM_BITS_8, 512),
  make_tuple(&av1_fht16x32_c, &av1_iht16x32_512_add_c, 7, AOM_BITS_8, 512),
  make_tuple(&av1_fht16x32_c, &av1_iht16x32_512_add_c, 8, AOM_BITS_8, 512),
  make_tuple(&av1_fht16x32_c, &av1_iht16x32_512_add_c, 9, AOM_BITS_8, 512),
  make_tuple(&av1_fht16x32_c, &av1_iht16x32_512_add_c, 10, AOM_BITS_8, 512),
  make_tuple(&av1_fht16x32_c, &av1_iht16x32_512_add_c, 11, AOM_BITS_8, 512),
  make_tuple(&av1_fht16x32_c, &av1_iht16x32_512_add_c, 12, AOM_BITS_8, 512),
  make_tuple(&av1_fht16x32_c, &av1_iht16x32_512_add_c, 13, AOM_BITS_8, 512),
  make_tuple(&av1_fht16x32_c, &av1_iht16x32_512_add_c, 14, AOM_BITS_8, 512),
  make_tuple(&av1_fht16x32_c, &av1_iht16x32_512_add_c, 15, AOM_BITS_8, 512)
#endif  // CONFIG_EXT_TX
};
INSTANTIATE_TEST_CASE_P(C, AV1Trans16x32HT,
                        ::testing::ValuesIn(kArrayHt16x32Param_c));

#if HAVE_SSE2
const Ht16x32Param kArrayHt16x32Param_sse2[] = {
  make_tuple(&av1_fht16x32_sse2, &av1_iht16x32_512_add_sse2, 0, AOM_BITS_8,
             512),
  make_tuple(&av1_fht16x32_sse2, &av1_iht16x32_512_add_sse2, 1, AOM_BITS_8,
             512),
  make_tuple(&av1_fht16x32_sse2, &av1_iht16x32_512_add_sse2, 2, AOM_BITS_8,
             512),
  make_tuple(&av1_fht16x32_sse2, &av1_iht16x32_512_add_sse2, 3, AOM_BITS_8,
             512),
#if CONFIG_EXT_TX
  make_tuple(&av1_fht16x32_sse2, &av1_iht16x32_512_add_sse2, 4, AOM_BITS_8,
             512),
  make_tuple(&av1_fht16x32_sse2, &av1_iht16x32_512_add_sse2, 5, AOM_BITS_8,
             512),
  make_tuple(&av1_fht16x32_sse2, &av1_iht16x32_512_add_sse2, 6, AOM_BITS_8,
             512),
  make_tuple(&av1_fht16x32_sse2, &av1_iht16x32_512_add_sse2, 7, AOM_BITS_8,
             512),
  make_tuple(&av1_fht16x32_sse2, &av1_iht16x32_512_add_sse2, 8, AOM_BITS_8,
             512),
  make_tuple(&av1_fht16x32_sse2, &av1_iht16x32_512_add_sse2, 9, AOM_BITS_8,
             512),
  make_tuple(&av1_fht16x32_sse2, &av1_iht16x32_512_add_sse2, 10, AOM_BITS_8,
             512),
  make_tuple(&av1_fht16x32_sse2, &av1_iht16x32_512_add_sse2, 11, AOM_BITS_8,
             512),
  make_tuple(&av1_fht16x32_sse2, &av1_iht16x32_512_add_sse2, 12, AOM_BITS_8,
             512),
  make_tuple(&av1_fht16x32_sse2, &av1_iht16x32_512_add_sse2, 13, AOM_BITS_8,
             512),
  make_tuple(&av1_fht16x32_sse2, &av1_iht16x32_512_add_sse2, 14, AOM_BITS_8,
             512),
  make_tuple(&av1_fht16x32_sse2, &av1_iht16x32_512_add_sse2, 15, AOM_BITS_8,
             512)
#endif  // CONFIG_EXT_TX
};
INSTANTIATE_TEST_CASE_P(SSE2, AV1Trans16x32HT,
                        ::testing::ValuesIn(kArrayHt16x32Param_sse2));
#endif  // HAVE_SSE2

}  // namespace