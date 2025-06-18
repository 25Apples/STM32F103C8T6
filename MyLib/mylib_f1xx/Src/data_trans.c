/**
 * @file data_trans.c
 * @brief Module truyền dữ liệu qua UART
 * @author Claude
 * @date 2025-04-11
 */

#include "data_trans.h"
#include <stdarg.h>
#include <stdio.h>   // Added for snprintf, vsnprintf
#include <string.h>  // Added for string functions (strcpy, strcat, etc.)

// Biến static để ánh xạ UART handle với đối tượng DataTrans
#define MAX_UART_HANDLERS 3
static struct {
    UART_HandleTypeDef* huart;
    DataTrans_t* dt;
} uart_mapping[MAX_UART_HANDLERS] = {0};

/**
 * @brief Khởi tạo module truyền dữ liệu
 * @param dt: Con trỏ đến đối tượng DataTrans
 * @param huart: Handle UART sẽ sử dụng
 * @return DataTransError: Mã lỗi
 */
DataTransError DataTrans_Init(DataTrans_t* dt, UART_HandleTypeDef* huart) {
    if (dt == NULL || huart == NULL) {
        return DATA_TRANS_ERROR_INVALID_PARAM;
    }

    // Khởi tạo các giá trị mặc định
    dt->config.huart = huart;
    dt->config.default_timeout = 1000;
    dt->config.add_newline = 1;
    strcpy(dt->config.newline_chars, "\r\n");
    dt->config.max_buffer_size = 256;
    dt->config.use_dma = 0;

    // Khởi tạo trạng thái
    dt->status.bytes_sent = 0;
    dt->status.tx_count = 0;
    dt->status.tx_errors = 0;
    dt->status.is_busy = 0;
    dt->status.last_tx_time = 0;

    dt->tx_complete_callback = NULL;
    dt->callback_user_data = NULL;
    dt->initialized = 1;

    // Đăng ký handle vào bảng ánh xạ
    for (int i = 0; i < MAX_UART_HANDLERS; i++) {
        if (uart_mapping[i].huart == NULL) {
            uart_mapping[i].huart = huart;
            uart_mapping[i].dt = dt;
            break;
        }
    }

    return DATA_TRANS_OK;
}

/**
 * @brief Cấu hình module truyền dữ liệu
 * @param dt: Con trỏ đến đối tượng DataTrans
 * @param config: Cấu hình mới
 * @return DataTransError: Mã lỗi
 */
DataTransError DataTrans_Config(DataTrans_t* dt, DataTransConfig_t* config) {
    if (dt == NULL || config == NULL) {
        return DATA_TRANS_ERROR_INVALID_PARAM;
    }

    if (!dt->initialized) {
        return DATA_TRANS_ERROR_NOT_INIT;
    }

    // Cập nhật cấu hình
    dt->config = *config;

    // Đảm bảo kích thước buffer hợp lý
    if (dt->config.max_buffer_size > sizeof(dt->buffer)) {
        dt->config.max_buffer_size = sizeof(dt->buffer);
    }

    return DATA_TRANS_OK;
}

/**
 * @brief Đặt callback khi truyền hoàn tất
 * @param dt: Con trỏ đến đối tượng DataTrans
 * @param callback: Hàm callback
 * @param user_data: Dữ liệu người dùng sẽ được truyền vào callback
 * @return DataTransError: Mã lỗi
 */
DataTransError DataTrans_SetCallback(DataTrans_t* dt, DataTransCallback callback, void* user_data) {
    if (dt == NULL) {
        return DATA_TRANS_ERROR_INVALID_PARAM;
    }

    if (!dt->initialized) {
        return DATA_TRANS_ERROR_NOT_INIT;
    }

    dt->tx_complete_callback = callback;
    dt->callback_user_data = user_data;

    return DATA_TRANS_OK;
}

/**
 * @brief Chuyển đổi dữ liệu sang chuỗi
 * @param data: Con trỏ đến dữ liệu
 * @param type: Loại dữ liệu
 * @param out: Buffer đầu ra
 * @param out_size: Kích thước buffer
 * @return char*: Con trỏ đến chuỗi kết quả hoặc NULL nếu lỗi
 */
static char* ConvertToString(void* data, DataType type, char* out, size_t out_size) {
    if (out == NULL || out_size == 0 || data == NULL) return NULL;

    switch (type) {
        case DATA_TYPE_UINT8: {
            uint8_t value = *(uint8_t*)data;
            snprintf(out, out_size, "%u", value);
            break;
        }
        case DATA_TYPE_INT8: {
            int8_t value = *(int8_t*)data;
            snprintf(out, out_size, "%d", value);
            break;
        }
        case DATA_TYPE_UINT16: {
            uint16_t value = *(uint16_t*)data;
            snprintf(out, out_size, "%u", value);
            break;
        }
        case DATA_TYPE_INT16: {
            int16_t value = *(int16_t*)data;
            snprintf(out, out_size, "%d", value);
            break;
        }
        case DATA_TYPE_UINT32: {
            uint32_t value = *(uint32_t*)data;
            // Use correct format specifier for uint32_t
            snprintf(out, out_size, "%lu", (unsigned long)value);
            break;
        }
        case DATA_TYPE_INT32: {
            int32_t value = *(int32_t*)data;
            // Use correct format specifier for int32_t
            snprintf(out, out_size, "%ld", (long)value);
            break;
        }
        case DATA_TYPE_FLOAT: {
            float value = *(float*)data;
            int int_part = (int)value;
            int frac_part = (int)((value - int_part) * 1000);  // 3 chữ số sau dấu .
            if (frac_part < 0) frac_part *= -1; // Tránh in dấu âm kép
            snprintf(out, out_size, "%d.%03d", int_part, frac_part);
            break;
        }
        case DATA_TYPE_STRING:
            // For strings, cast data to char* directly
            snprintf(out, out_size, "%s", (char*)data);
            break;
        case DATA_TYPE_HEX: {
            uint8_t value = *(uint8_t*)data;
            snprintf(out, out_size, "0x%02X", value);
            break;
        }
        case DATA_TYPE_BINARY: {
            uint8_t val = *(uint8_t*)data;
            char temp[9];
            for (int i = 0; i < 8; i++) {
                temp[7-i] = (val & (1 << i)) ? '1' : '0';
            }
            temp[8] = '\0';
            snprintf(out, out_size, "0b%s", temp);
            break;
        }
        case DATA_TYPE_ARRAY:
            // Xử lý riêng bên ngoài
            snprintf(out, out_size, "ARRAY");
            break;
        default:
            snprintf(out, out_size, "UNKNOWN");
            break;
    }

    return out;
}

/**
 * @brief Gửi dữ liệu qua UART
 * @param dt: Con trỏ đến đối tượng DataTrans
 * @param data: Con trỏ đến dữ liệu cần gửi
 * @param type: Loại dữ liệu
 * @param timeout: Timeout (ms)
 * @return DataTransError: Mã lỗi
 */
DataTransError DataTrans_SendData(DataTrans_t* dt, void* data, DataType type, uint32_t timeout) {
    if (dt == NULL || data == NULL) {
        return DATA_TRANS_ERROR_INVALID_PARAM;
    }

    if (!dt->initialized) {
        return DATA_TRANS_ERROR_NOT_INIT;
    }

    if (dt->status.is_busy) {
        return DATA_TRANS_ERROR_BUSY;
    }

    // Nếu timeout = 0, sử dụng timeout mặc định
    if (timeout == 0) {
        timeout = dt->config.default_timeout;
    }

    char* result = ConvertToString(data, type, dt->buffer, dt->config.max_buffer_size);
    if (result == NULL) {
        dt->status.tx_errors++;
        return DATA_TRANS_ERROR_INVALID_PARAM;
    }

    // Thêm ký tự xuống dòng nếu được cấu hình
    if (dt->config.add_newline) {
        size_t len = strlen(result);
        size_t nl_len = strlen(dt->config.newline_chars);

        if (len + nl_len < dt->config.max_buffer_size) {
            strcat(result, dt->config.newline_chars);
        }
    }

    HAL_StatusTypeDef hal_status;
    dt->status.is_busy = 1;

    if (dt->config.use_dma) {
        hal_status = HAL_UART_Transmit_DMA(dt->config.huart, (uint8_t*)result, strlen(result));
    } else {
        hal_status = HAL_UART_Transmit(dt->config.huart, (uint8_t*)result, strlen(result), timeout);

        // Xử lý hoàn thành ngay lập tức nếu không dùng DMA
        dt->status.is_busy = 0;
        dt->status.bytes_sent += strlen(result);
        dt->status.tx_count++;
        dt->status.last_tx_time = HAL_GetTick();

        if (dt->tx_complete_callback != NULL) {
            dt->tx_complete_callback((hal_status == HAL_OK) ? 1 : 0, dt->callback_user_data);
        }
    }

    if (hal_status != HAL_OK) {
        dt->status.tx_errors++;
        dt->status.is_busy = 0;
        return DATA_TRANS_ERROR_HAL;
    }

    return DATA_TRANS_OK;
}

/**
 * @brief Gửi chuỗi qua UART
 * @param dt: Con trỏ đến đối tượng DataTrans
 * @param str: Chuỗi cần gửi
 * @param timeout: Timeout (ms)
 * @return DataTransError: Mã lỗi
 */
DataTransError DataTrans_SendString(DataTrans_t* dt, const char* str, uint32_t timeout) {
    if (dt == NULL || str == NULL) {
        return DATA_TRANS_ERROR_INVALID_PARAM;
    }

    if (!dt->initialized) {
        return DATA_TRANS_ERROR_NOT_INIT;
    }

    if (dt->status.is_busy) {
        return DATA_TRANS_ERROR_BUSY;
    }

    // Nếu timeout = 0, sử dụng timeout mặc định
    if (timeout == 0) {
        timeout = dt->config.default_timeout;
    }

    size_t str_len = strlen(str);
    if (str_len >= dt->config.max_buffer_size) {
        str_len = dt->config.max_buffer_size - 1;
    }

    // Sao chép chuỗi vào buffer
    strncpy(dt->buffer, str, str_len);
    dt->buffer[str_len] = '\0';

    // Thêm ký tự xuống dòng nếu được cấu hình
    if (dt->config.add_newline) {
        size_t nl_len = strlen(dt->config.newline_chars);

        if (str_len + nl_len < dt->config.max_buffer_size) {
            strcat(dt->buffer, dt->config.newline_chars);
        }
    }

    HAL_StatusTypeDef hal_status;
    dt->status.is_busy = 1;

    if (dt->config.use_dma) {
        hal_status = HAL_UART_Transmit_DMA(dt->config.huart, (uint8_t*)dt->buffer, strlen(dt->buffer));
    } else {
        hal_status = HAL_UART_Transmit(dt->config.huart, (uint8_t*)dt->buffer, strlen(dt->buffer), timeout);

        // Xử lý hoàn thành ngay lập tức nếu không dùng DMA
        dt->status.is_busy = 0;
        dt->status.bytes_sent += strlen(dt->buffer);
        dt->status.tx_count++;
        dt->status.last_tx_time = HAL_GetTick();

        if (dt->tx_complete_callback != NULL) {
            dt->tx_complete_callback((hal_status == HAL_OK) ? 1 : 0, dt->callback_user_data);
        }
    }

    if (hal_status != HAL_OK) {
        dt->status.tx_errors++;
        dt->status.is_busy = 0;
        return DATA_TRANS_ERROR_HAL;
    }

    return DATA_TRANS_OK;
}

/**
 * @brief Gửi dữ liệu dạng hex qua UART
 * @param dt: Con trỏ đến đối tượng DataTrans
 * @param data: Con trỏ đến dữ liệu
 * @param len: Độ dài dữ liệu (byte)
 * @param timeout: Timeout (ms)
 * @return DataTransError: Mã lỗi
 */
DataTransError DataTrans_SendHexData(DataTrans_t* dt, uint8_t* data, uint16_t len, uint32_t timeout) {
    if (dt == NULL || data == NULL || len == 0) {
        return DATA_TRANS_ERROR_INVALID_PARAM;
    }

    if (!dt->initialized) {
        return DATA_TRANS_ERROR_NOT_INIT;
    }

    if (dt->status.is_busy) {
        return DATA_TRANS_ERROR_BUSY;
    }

    // Nếu timeout = 0, sử dụng timeout mặc định
    if (timeout == 0) {
        timeout = dt->config.default_timeout;
    }

    // Kiểm tra kích thước buffer: mỗi byte cần 2 ký tự hex + cách và xuống dòng
    if (len * 3 + 2 > dt->config.max_buffer_size) {
        return DATA_TRANS_ERROR_BUFFER_OVERFLOW;
    }

    // Chuyển đổi mảng byte thành chuỗi hex
    size_t pos = 0;
    for (uint16_t i = 0; i < len; i++) {
        pos += snprintf(dt->buffer + pos, dt->config.max_buffer_size - pos, "%02X ", data[i]);
    }

    // Thêm ký tự xuống dòng nếu được cấu hình
    if (dt->config.add_newline) {
        strcat(dt->buffer, dt->config.newline_chars);
    }

    HAL_StatusTypeDef hal_status;
    dt->status.is_busy = 1;

    if (dt->config.use_dma) {
        hal_status = HAL_UART_Transmit_DMA(dt->config.huart, (uint8_t*)dt->buffer, strlen(dt->buffer));
    } else {
        hal_status = HAL_UART_Transmit(dt->config.huart, (uint8_t*)dt->buffer, strlen(dt->buffer), timeout);

        // Xử lý hoàn thành ngay lập tức nếu không dùng DMA
        dt->status.is_busy = 0;
        dt->status.bytes_sent += strlen(dt->buffer);
        dt->status.tx_count++;
        dt->status.last_tx_time = HAL_GetTick();

        if (dt->tx_complete_callback != NULL) {
            dt->tx_complete_callback((hal_status == HAL_OK) ? 1 : 0, dt->callback_user_data);
        }
    }

    if (hal_status != HAL_OK) {
        dt->status.tx_errors++;
        dt->status.is_busy = 0;
        return DATA_TRANS_ERROR_HAL;
    }

    return DATA_TRANS_OK;
}

/**
 * @brief Gửi mảng dữ liệu qua UART
 * @param dt: Con trỏ đến đối tượng DataTrans
 * @param data: Con trỏ đến mảng dữ liệu
 * @param len: Số phần tử trong mảng
 * @param element_type: Loại dữ liệu của mỗi phần tử
 * @param timeout: Timeout (ms)
 * @return DataTransError: Mã lỗi
 */
DataTransError DataTrans_SendArray(DataTrans_t* dt, void* data, uint16_t len, DataType element_type, uint32_t timeout) {
    if (dt == NULL || data == NULL || len == 0) {
        return DATA_TRANS_ERROR_INVALID_PARAM;
    }

    if (!dt->initialized) {
        return DATA_TRANS_ERROR_NOT_INIT;
    }

    if (dt->status.is_busy) {
        return DATA_TRANS_ERROR_BUSY;
    }

    // Nếu timeout = 0, sử dụng timeout mặc định
    if (timeout == 0) {
        timeout = dt->config.default_timeout;
    }

    // Xác định kích thước mỗi phần tử
    uint8_t element_size;
    switch (element_type) {
        case DATA_TYPE_UINT8:
        case DATA_TYPE_INT8:
        case DATA_TYPE_HEX:
        case DATA_TYPE_BINARY:
            element_size = 1;
            break;
        case DATA_TYPE_UINT16:
        case DATA_TYPE_INT16:
            element_size = 2;
            break;
        case DATA_TYPE_UINT32:
        case DATA_TYPE_INT32:
        case DATA_TYPE_FLOAT:
            element_size = 4;
            break;
        default:
            return DATA_TRANS_ERROR_INVALID_PARAM;
    }

    // Tạo chuỗi kết quả với định dạng [x, y, z]
    size_t pos = 0;
    pos += snprintf(dt->buffer + pos, dt->config.max_buffer_size - pos, "[");

    char temp[32];
    uint8_t* ptr = (uint8_t*)data;

    for (uint16_t i = 0; i < len; i++) {
        // Chuyển đổi phần tử hiện tại sang chuỗi
        ConvertToString(ptr, element_type, temp, sizeof(temp));

        // Thêm dấu phẩy nếu không phải phần tử cuối cùng
        if (i < len - 1) {
            pos += snprintf(dt->buffer + pos, dt->config.max_buffer_size - pos, "%s, ", temp);
        } else {
            pos += snprintf(dt->buffer + pos, dt->config.max_buffer_size - pos, "%s]", temp);
        }

        // Kiểm tra buffer overflow
        if (pos >= dt->config.max_buffer_size - 1) {
            dt->buffer[dt->config.max_buffer_size - 4] = '.';
            dt->buffer[dt->config.max_buffer_size - 3] = '.';
            dt->buffer[dt->config.max_buffer_size - 2] = ']';
            dt->buffer[dt->config.max_buffer_size - 1] = '\0';
            break;
        }

        // Di chuyển con trỏ đến phần tử tiếp theo
        ptr += element_size;
    }

    // Thêm ký tự xuống dòng nếu được cấu hình
    if (dt->config.add_newline) {
        size_t nl_len = strlen(dt->config.newline_chars);

        if (pos + nl_len < dt->config.max_buffer_size) {
            strcat(dt->buffer, dt->config.newline_chars);
        }
    }

    HAL_StatusTypeDef hal_status;
    dt->status.is_busy = 1;

    if (dt->config.use_dma) {
        hal_status = HAL_UART_Transmit_DMA(dt->config.huart, (uint8_t*)dt->buffer, strlen(dt->buffer));
    } else {
        hal_status = HAL_UART_Transmit(dt->config.huart, (uint8_t*)dt->buffer, strlen(dt->buffer), timeout);

        // Xử lý hoàn thành ngay lập tức nếu không dùng DMA
        dt->status.is_busy = 0;
        dt->status.bytes_sent += strlen(dt->buffer);
        dt->status.tx_count++;
        dt->status.last_tx_time = HAL_GetTick();

        if (dt->tx_complete_callback != NULL) {
            dt->tx_complete_callback((hal_status == HAL_OK) ? 1 : 0, dt->callback_user_data);
        }
    }

    if (hal_status != HAL_OK) {
        dt->status.tx_errors++;
        dt->status.is_busy = 0;
        return DATA_TRANS_ERROR_HAL;
    }

    return DATA_TRANS_OK;
}

/**
 * @brief Gửi dữ liệu định dạng printf qua UART
 * @param dt: Con trỏ đến đối tượng DataTrans
 * @param timeout: Timeout (ms)
 * @param format: Chuỗi định dạng
 * @param ...: Các tham số biến đổi
 * @return DataTransError: Mã lỗi
 */
DataTransError DataTrans_Printf(DataTrans_t* dt, uint32_t timeout, const char* format, ...) {
    if (dt == NULL || format == NULL) {
        return DATA_TRANS_ERROR_INVALID_PARAM;
    }

    if (!dt->initialized) {
        return DATA_TRANS_ERROR_NOT_INIT;
    }

    if (dt->status.is_busy) {
        return DATA_TRANS_ERROR_BUSY;
    }

    // Nếu timeout = 0, sử dụng timeout mặc định
    if (timeout == 0) {
        timeout = dt->config.default_timeout;
    }

    va_list args;
    va_start(args, format);
    vsnprintf(dt->buffer, dt->config.max_buffer_size, format, args);
    va_end(args);

    // Thêm ký tự xuống dòng nếu được cấu hình
    if (dt->config.add_newline) {
        size_t len = strlen(dt->buffer);
        size_t nl_len = strlen(dt->config.newline_chars);

        if (len + nl_len < dt->config.max_buffer_size) {
            strcat(dt->buffer, dt->config.newline_chars);
        }
    }

    HAL_StatusTypeDef hal_status;
    dt->status.is_busy = 1;

    if (dt->config.use_dma) {
        hal_status = HAL_UART_Transmit_DMA(dt->config.huart, (uint8_t*)dt->buffer, strlen(dt->buffer));
    } else {
        hal_status = HAL_UART_Transmit(dt->config.huart, (uint8_t*)dt->buffer, strlen(dt->buffer), timeout);

        // Xử lý hoàn thành ngay lập tức nếu không dùng DMA
        dt->status.is_busy = 0;
        dt->status.bytes_sent += strlen(dt->buffer);
        dt->status.tx_count++;
        dt->status.last_tx_time = HAL_GetTick();

        if (dt->tx_complete_callback != NULL) {
            dt->tx_complete_callback((hal_status == HAL_OK) ? 1 : 0, dt->callback_user_data);
        }
    }

    if (hal_status != HAL_OK) {
        dt->status.tx_errors++;
        dt->status.is_busy = 0;
        return DATA_TRANS_ERROR_HAL;
    }

    return DATA_TRANS_OK;
}

/**
 * @brief Xử lý hoàn thành truyền DMA
 * @param huart: UART handle
 */
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart) {
    // Tìm đối tượng DataTrans tương ứng
    for (int i = 0; i < MAX_UART_HANDLERS; i++) {
        if (uart_mapping[i].huart == huart) {
            DataTrans_t* dt = uart_mapping[i].dt;
            if (dt != NULL) {
                dt->status.is_busy = 0;
                dt->status.bytes_sent += strlen(dt->buffer);
                dt->status.tx_count++;
                dt->status.last_tx_time = HAL_GetTick();

                if (dt->tx_complete_callback != NULL) {
                    dt->tx_complete_callback(1, dt->callback_user_data);
                }
            }
            break;
        }
    }
}

/**
 * @brief Lấy thông tin trạng thái của DataTrans
 * @param dt: Con trỏ đến đối tượng DataTrans
 * @param status: Con trỏ đến biến lưu trạng thái
 * @return DataTransError: Mã lỗi
 */
DataTransError DataTrans_GetStatus(DataTrans_t* dt, DataTransStatus_t* status) {
    if (dt == NULL || status == NULL) {
        return DATA_TRANS_ERROR_INVALID_PARAM;
    }

    if (!dt->initialized) {
        return DATA_TRANS_ERROR_NOT_INIT;
    }

    *status = dt->status;
    return DATA_TRANS_OK;
}

/**
 * @brief Đặt lại các biến thống kê
 * @param dt: Con trỏ đến đối tượng DataTrans
 * @return DataTransError: Mã lỗi
 */
DataTransError DataTrans_ResetStats(DataTrans_t* dt) {
    if (dt == NULL) {
        return DATA_TRANS_ERROR_INVALID_PARAM;
    }

    if (!dt->initialized) {
        return DATA_TRANS_ERROR_NOT_INIT;
    }

    dt->status.bytes_sent = 0;
    dt->status.tx_count = 0;
    dt->status.tx_errors = 0;
    dt->status.last_tx_time = 0;

    return DATA_TRANS_OK;
}
