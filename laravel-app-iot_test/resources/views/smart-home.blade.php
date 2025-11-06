<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Điều Khiển LED Qua Laravel & MQTT</title>
    <link href="https://cdn.jsdelivr.net/npm/bootstrap@5.3.3/dist/css/bootstrap.min.css" rel="stylesheet">
    <meta name="csrf-token" content="{{ csrf_token() }}"> </head>
<body>
    <div class="container mt-5">
        <h1 class="mb-4 text-primary">💡 Điều Khiển Thiết Bị Nhà Thông Minh</h1>
        <p class="lead">Laravel (Front-end) ➡️ Node.js Gateway (API) ➡️ MQTT ➡️ ESP32 (LED)</p>
        
        <hr>

        <div class="row">
            <div class="col-md-6 mb-4">
                <div class="card shadow-sm">
                    <div class="card-header bg-warning text-white">
                        <h4 class="mb-0">Chế độ Thủ công (MANUAL)</h4>
                    </div>
                    <div class="card-body text-center">
                        <p class="card-text">Bật/Tắt LED trực tiếp.</p>
                        <button class="btn btn-success btn-lg mx-2 control-btn" data-command="ON" data-mode="MANUAL">BẬT LED</button>
                        <button class="btn btn-danger btn-lg mx-2 control-btn" data-command="OFF" data-mode="MANUAL">TẮT LED</button>
                    </div>
                </div>
            </div>

            <div class="col-md-6 mb-4">
                <div class="card shadow-sm">
                    <div class="card-header bg-info text-white">
                        <h4 class="mb-0">Chế độ Tự động (AUTO)</h4>
                    </div>
                    <div class="card-body text-center">
                        <p class="card-text">Để Node.js tự điều khiển theo cảm biến (LDR).</p>
                        <button class="btn btn-primary btn-lg control-btn" data-mode="AUTO">KÍCH HOẠT AUTO</button>
                    </div>
                </div>
            </div>
        </div>
        
        <hr>
        
        <div class="mt-4">
            <h4>Trạng thái và Dữ liệu Cảm biến</h4>
            <div class="alert alert-secondary" id="status-display">
                <strong>Trạng thái LED hiện tại:</strong> Đang tải... | <strong>Chế độ:</strong> Đang tải...
            </div>
            <div class="alert alert-light" id="sensor-data-display">
                Nhiệt độ: --°C | Độ ẩm: --% | Ánh sáng: --
            </div>
        </div>

    </div>

    <script src="https://code.jquery.com/jquery-3.7.1.min.js"></script>
    <script>
        $(document).ready(function() {
            // Lấy CSRF token
            $.ajaxSetup({
                headers: {
                    'X-CSRF-TOKEN': $('meta[name="csrf-token"]').attr('content')
                }
            });

            // Hàm gửi lệnh AJAX đến Controller Laravel
            function sendCommand(mode, command = null) {
                const payload = { mode: mode };
                if (command) {
                    payload.command = command;
                }

                $('#status-display').removeClass('alert-success alert-danger alert-secondary').addClass('alert-warning').html('<strong>Đang gửi lệnh...</strong>');
                
                $.ajax({
                    url: '/led/control', // Route Laravel đã định nghĩa
                    type: 'POST',
                    contentType: 'application/json',
                    data: JSON.stringify(payload),
                    success: function(response) {
                        $('#status-display')
                            .removeClass('alert-warning alert-danger')
                            .addClass('alert-success')
                            .html(`<strong>Thành công:</strong> ${response.message} | Node.js trả lời: ${response.details.message}`);
                        // Cập nhật trạng thái sau khi gửi lệnh thành công
                        getLedStatus(); 
                    },
                    error: function(xhr) {
                        const errorMsg = xhr.responseJSON ? xhr.responseJSON.message : 'Lỗi kết nối hoặc server Node.js không khả dụng.';
                        $('#status-display')
                            .removeClass('alert-warning alert-success')
                            .addClass('alert-danger')
                            .html(`<strong>Lỗi:</strong> ${errorMsg}`);
                    }
                });
            }

            // Hàm lấy trạng thái từ Laravel Controller (Controller lại gọi Node.js Gateway)
            function getLedStatus() {
                $.ajax({
                    url: '/led/status', // Route Laravel đã định nghĩa
                    type: 'GET',
                    success: function(response) {
                        const data = response.data;
                        const ledState = data.led_state;
                        const mode = data.mode;
                        const sensor = data.sensor;
                        
                        // Hiển thị trạng thái
                        $('#status-display')
                            .removeClass('alert-warning alert-success alert-danger')
                            .addClass(mode === 'AUTO' ? 'alert-info' : (ledState === 'ON' ? 'alert-success' : 'alert-danger'))
                            .html(`<strong>Trạng thái LED hiện tại:</strong> ${ledState} | <strong>Chế độ:</strong> ${mode}`);
                        
                        // Hiển thị dữ liệu cảm biến
                        $('#sensor-data-display').html(`
                            Nhiệt độ: <strong>${sensor.temperature}°C</strong> | 
                            Độ ẩm: <strong>${sensor.humidity}%</strong> 
                            ${sensor.light ? '| Ánh sáng: <strong>' + sensor.light + '</strong>' : ''}
                        `);
                    },
                    error: function() {
                        $('#status-display').html('<strong>Không thể lấy trạng thái. Kiểm tra Node.js Gateway.</strong>');
                    }
                });
            }


            // Xử lý sự kiện khi click nút
            $('.control-btn').on('click', function() {
                const mode = $(this).data('mode');
                const command = $(this).data('command');
                sendCommand(mode, command);
            });
            
            // Lấy trạng thái lần đầu và thiết lập lặp lại
            getLedStatus();
            setInterval(getLedStatus, 5000); // Lấy trạng thái mỗi 5 giây
        });
    </script>
</body>
</html>