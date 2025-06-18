/**
 * @file data_trans.h
 * @brief Module truyền dữ liệu qua UART
 * @author Claude
 * @date 2025-04-11
 */

#ifndef DATA_TRANS_H
#define DATA_TRANS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f1xx_hal.h"
#include <stdint.h>
#include <stddef.h>
#include <string.h>

/**
 * @brief Enum mã lỗi của module
 */
typedef enum {
    DATA_TRANS_OK = 0,                 /**< Không có lỗi */
    DATA_TRANS_ERROR_INVALID_PARAM,    /**< Tham số không hợp lệ */
    DATA_TRANS_ERROR_NOT_INIT,         /**< Module chưa khởi tạo */
    DATA_TRANS_ERROR_BUSY,             /**< Module đang bận */
    DATA_TRANS_ERROR_TIMEOUT,          /**< Hết thời gian chờ */
    DATA_TRANS_ERROR_HAL,              /**< Lỗi từ HAL */
    DATA_TRANS_ERROR_BUFFER_OVERFLOW   /**< Tràn buffer */
} DataTransError;

/**
 * @brief Enum loại dữ liệu
 */
typedef enum {
    DATA_TYPE_UINT8,      /**< Số nguyên không dấu 8-bit */
    DATA_TYPE_INT8,       /**< Số nguyên có dấu 8-bit */
    DATA_TYPE_UINT16,     /**< Số nguyên không dấu 16-bit */
    DATA_TYPE_INT16,      /**< Số nguyên có dấu 16-bit */
    DATA_TYPE_UINT32,     /**< Số nguyên không dấu 32-bit */
    DATA_TYPE_INT32,      /**< Số nguyên có dấu 32-bit */
    DATA_TYPE_FLOAT,      /**< Số thực 32-bit */
    DATA_TYPE_STRING,     /**< Chuỗi ký tự */
    DATA_TYPE_HEX,        /**< Dữ liệu hex */
    DATA_TYPE_BINARY,     /**< Dữ liệu nhị phân */
    DATA_TYPE_ARRAY       /**< Mảng dữ liệu */
} DataType;

/**
 * @brief Enum định dạng hiển thị số thực
 */
typedef enum {
    FLOAT_FORMAT_FIXED,   /**< Định dạng cố định (%.nf) */
    FLOAT_FORMAT_EXP,     /**< Định dạng số mũ (%.ne) */
    FLOAT_FORMAT_AUTO     /**< Định dạng tự động (%.ng) */
} FloatFormat;

/**
 * @brief Struct cấu hình module
 */
typedef struct {
    UART_HandleTypeDef* huart;       /**< Handle UART */
    uint32_t default_timeout;        /**< Timeout mặc định (ms) */
    uint8_t add_newline;             /**< Thêm ký tự xuống dòng (1: có, 0: không) */
    char newline_chars[4];           /**< Ký tự xuống dòng (vd: "\r\n") */
    uint16_t max_buffer_size;        /**< Kích thước tối đa của buffer */
    uint8_t use_dma;                 /**< Sử dụng DMA (1: có, 0: không) */
} DataTransConfig_t;

/**
 * @brief Struct trạng thái module
 */
typedef struct {
    uint32_t bytes_sent;            /**< Tổng số byte đã gửi */
    uint32_t tx_count;              /**< Số lần truyền */
    uint32_t tx_errors;             /**< Số lỗi truyền */
    uint8_t is_busy;                /**< Trạng thái bận (1: bận, 0: rảnh) */
    uint32_t last_tx_time;          /**< Thời điểm truyền cuối cùng */
} DataTransStatus_t;

/**
 * @brief Kiểu hàm callback
 * @param success: Kết quả truyền (1: thành công, 0: thất bại)
 * @param user_data: Dữ liệu người dùng
 */
typedef void (*DataTransCallback)(uint8_t success, void* user_data);

/**
 * @brief Struct đối tượng DataTrans
 */
typedef struct {
    DataTransConfig_t config;              /**< Cấu hình */
    DataTransStatus_t status;              /**< Trạng thái */
    DataTransCallback tx_complete_callback; /**< Callback hoàn thành */
    void* callback_user_data;              /**< Dữ liệu cho callback */
    uint8_t initialized;                   /**< Đã khởi tạo chưa */
    char buffer[512];                      /**< Buffer nội bộ */
} DataTrans_t;

/**
 * @brief Khởi tạo module truyền dữ liệu
 * @param dt: Con trỏ đến đối tượng DataTrans
 * @param huart: Handle UART sẽ sử dụng
 * @return DataTransError: Mã lỗi
 */
DataTransError DataTrans_Init(DataTrans_t* dt, UART_HandleTypeDef* huart);

/**
 * @brief Cấu hình module truyền dữ liệu
 * @param dt: Con trỏ đến đối tượng DataTrans
 * @param config: Cấu hình mới
 * @return DataTransError: Mã lỗi
 */
DataTransError DataTrans_Config(DataTrans_t* dt, DataTransConfig_t* config);

/**
 * @brief Đặt callback khi truyền hoàn tất
 * @param dt: Con trỏ đến đối tượng DataTrans
 * @param callback: Hàm callback
 * @param user_data: Dữ liệu người dùng sẽ được truyền vào callback
 * @return DataTransError: Mã lỗi
 */
DataTransError DataTrans_SetCallback(DataTrans_t* dt, DataTransCallback callback, void* user_data);

/**
 * @brief Gửi dữ liệu qua UART
 * @param dt: Con trỏ đến đối tượng DataTrans
 * @param data: Con trỏ đến dữ liệu cần gửi
 * @param type: Loại dữ liệu
 * @param timeout: Timeout (ms), 0 để sử dụng timeout mặc định
 * @return DataTransError: Mã lỗi
 */
DataTransError DataTrans_SendData(DataTrans_t* dt, void* data, DataType type, uint32_t timeout);

/**
 * @brief Gửi chuỗi qua UART
 * @param dt: Con trỏ đến đối tượng DataTrans
 * @param str: Chuỗi cần gửi
 * @param timeout: Timeout (ms), 0 để sử dụng timeout mặc định
 * @return DataTransError: Mã lỗi
 */
DataTransError DataTrans_SendString(DataTrans_t* dt, const char* str, uint32_t timeout);

/**
 * @brief Gửi dữ liệu dạng hex qua UART
 * @param dt: Con trỏ đến đối tượng DataTrans
 * @param data: Con trỏ đến dữ liệu
 * @param len: Độ dài dữ liệu (byte)
 * @param timeout: Timeout (ms), 0 để sử dụng timeout mặc định
 * @return DataTransError: Mã lỗi
 */
DataTransError DataTrans_SendHexData(DataTrans_t* dt, uint8_t* data, uint16_t len, uint32_t timeout);

/**
 * @brief Gửi mảng dữ liệu qua UART
 * @param dt: Con trỏ đến đối tượng DataTrans
 * @param data: Con trỏ đến mảng dữ liệu
 * @param len: Số phần tử trong mảng
 * @param element_type: Loại dữ liệu của mỗi phần tử
 * @param timeout: Timeout (ms), 0 để sử dụng timeout mặc định
 * @return DataTransError: Mã lỗi
 */
DataTransError DataTrans_SendArray(DataTrans_t* dt, void* data, uint16_t len, DataType element_type, uint32_t timeout);

/**
 * @brief Gửi số thực với độ chính xác tùy chỉnh qua UART
 * @param dt: Con trỏ đến đối tượng DataTrans
 * @param value: Giá trị số thực
 * @param precision: Số chữ số thập phân (0-6)
 * @param format: Định dạng hiển thị
 * @param timeout: Timeout (ms), 0 để sử dụng timeout mặc định
 * @return DataTransError: Mã lỗi
 */
DataTransError DataTrans_SendFloat(DataTrans_t* dt, float value, uint8_t precision, FloatFormat format, uint32_t timeout);

/**
 * @brief Gửi dữ liệu dạng printf qua UART
 * @param dt: Con trỏ đến đối tượng DataTrans
 * @param timeout: Timeout (ms), 0 để sử dụng timeout mặc định
 * @param format: Chuỗi định dạng
 * @param ...: Các tham số biến đổi
 * @return DataTransError: Mã lỗi
 */
DataTransError DataTrans_Printf(DataTrans_t* dt, uint32_t timeout, const char* format, ...);

/**
 * @brief Kiểm tra trạng thái bận của module
 * @param dt: Con trỏ đến đối tượng DataTrans
 * @return uint8_t: 1 nếu đang bận, 0 nếu đang rảnh
 */
uint8_t DataTrans_IsBusy(DataTrans_t* dt);

/**
 * @brief Lấy thông tin trạng thái hiện tại
 * @param dt: Con trỏ đến đối tượng DataTrans
 * @param status: Con trỏ đến biến nhận thông tin trạng thái
 * @return DataTransError: Mã lỗi
 */
DataTransError DataTrans_GetStatus(DataTrans_t* dt, DataTransStatus_t* status);

/**
 * @brief Đặt lại bộ đếm thống kê
 * @param dt: Con trỏ đến đối tượng DataTrans
 * @return DataTransError: Mã lỗi
 */
DataTransError DataTrans_ResetStats(DataTrans_t* dt);

/**
 * @brief Xử lý callback cho truyền DMA hoàn tất
 * @note Cần gọi hàm này trong callback HAL_UART_TxCpltCallback
 * @param huart: Handle UART đã hoàn tất truyền
 */
void DataTrans_HandleTxComplete(UART_HandleTypeDef* huart);

#ifdef __cplusplus
}
#endif

#endif /* DATA_TRANS_H */
