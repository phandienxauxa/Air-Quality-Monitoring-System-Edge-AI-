#include "edge-impulse-sdk/classifier/ei_run_classifier.h"
#include <string.h>

/* =========================================================================
 * KHAI BÁO CÁC HÀM VÔ TRI (DUMMY FUNCTIONS) ĐỂ TRÁNH WARNING COMPILER
 * Giữ nguyên các hàm này để Edge Impulse không báo lỗi Undefined Reference
 * ========================================================================= */
// 1. Đưa ei_printf ra NGOÀI khối extern "C" để khớp với khai báo của Edge Impulse
void ei_printf(const char *format, ...) {
    va_list myargs;
    va_start(myargs, format);
    // Để trống để tiết kiệm tài nguyên bộ nhớ
    va_end(myargs);
}

// 1. CÁC MẢNG TIỀN XỬ LÝ (SCALING)
// Dùng để đưa dữ liệu thô về đúng thang đo mà mô hình đã học
const float SCALER_MEAN[] = {1.316391f, 0.078917f, 0.148365f, 90.028649f, 38.401188f, 53.825887f, 0.000719f, 0.000194f, 0.000363f, 0.008044f, 0.006719f, -0.024178f};
const float SCALER_STD[]  = {0.407336f, 0.125458f, 0.235859f, 22.328169f, 1.445634f, 4.769430f, 0.209214f, 0.112543f, 0.211578f, 8.764516f, 0.134505f, 1.098872f};

// Biến toàn cục lưu trữ 12 thông số đã được chuẩn hóa
float features_buffer[EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE];

// 2. Hàm Callback cung cấp dữ liệu cho thư viện Edge Impulse
int ei_get_data(size_t offset, size_t length, float *out_ptr) {
    memcpy(out_ptr, features_buffer + offset, length * sizeof(float));
    return 0;
}

// 3. Hàm Wrapper cho dự án Edge AI trên kit RA6M5
// Dùng extern "C" để C-compiler trong hal_entry.c có thể gọi được hàm này
extern "C" float run_iaq_prediction(float* sensor_data) {

    // BƯỚC QUAN TRỌNG: CHUẨN HÓA DỮ LIỆU ĐẦU VÀO
    // Công thức: x_new = (x_raw - mean) / std
    for (size_t i = 0; i < EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE; i++) {
        // Thêm kiểm tra an toàn chia cho 0 (Best practice cho code nhúng C/C++)
        if (SCALER_STD[i] > 0.000001f) {
            features_buffer[i] = (sensor_data[i] - SCALER_MEAN[i]) / SCALER_STD[i];
        } else {
            features_buffer[i] = 0.0f;
        }
    }

    // Đóng gói tín hiệu
    signal_t features_signal;
    features_signal.total_length = EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE;
    features_signal.get_data = &ei_get_data;

    // Khởi tạo kết quả
    ei_impulse_result_t result = { 0 };

    // KÍCH HOẠT SUY LUẬN
    // Tham số cuối là 'false' để tiết kiệm Stack, tránh lỗi treo mạch đã gặp
    EI_IMPULSE_ERROR res = run_classifier(&features_signal, &result, false);

    if (res == EI_IMPULSE_OK) {
        // Trả về kết quả dự đoán IAQ
        return result.classification[0].value;
    }

    // Trả về mã lỗi nếu suy luận thất bại
    return -999.0f;
}
