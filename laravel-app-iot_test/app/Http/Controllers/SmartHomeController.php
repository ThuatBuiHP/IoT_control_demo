<?php

namespace App\Http\Controllers;
use Illuminate\Http\Request;
use GuzzleHttp\Client;
use GuzzleHttp\Exception\RequestException;
use App\Http\Controllers\Controller;

    class SmartHomeController extends Controller
{
    // Địa chỉ của Node.js Server Gateway
    protected $gatewayUrl = 'http://192.168.126.1:5000/api/led/control';
    
    public function controlLed(Request $request)
    {
        // Xác thực dữ liệu
        $request->validate([
            'mode' => 'required|in:MANUAL,AUTO',
            'command' => 'required_if:mode,MANUAL|in:ON,OFF,', // Chỉ cần command khi MANUAL
        ]);

        $mode = $request->input('mode');
        $command = $request->input('command');

        // Chuẩn bị dữ liệu gửi đến Node.js
        $dataToSend = ['mode' => $mode];
        if ($mode === 'MANUAL') {
            $dataToSend['command'] = $command;
        }

        try {
            $client = new Client();
            
            // Gửi lệnh POST đến Node.js Gateway
            $response = $client->post($this->gatewayUrl, [
                'json' => $dataToSend,
            ]);

            // Xử lý phản hồi
            $result = json_decode($response->getBody()->getContents());

            if ($response->getStatusCode() === 200 && $result->status === 'success') {
                return response()->json(['message' => 'Command sent successfully via Node.js Gateway.', 'details' => $result], 200);
            }

            return response()->json(['message' => 'Gateway returned an error.', 'details' => $result], 500);

        } catch (RequestException $e) {
            // Xử lý lỗi kết nối (ví dụ: Node.js server bị down)
            return response()->json(['message' => 'Could not connect to Node.js Gateway.', 'error' => $e->getMessage()], 503);
        }
    }
    
    // (Tùy chọn) Hàm lấy trạng thái
    public function getLedStatus()
    {
        try {
            $client = new Client();
            // Gửi yêu cầu GET_STATE đến Node.js Gateway
            $response = $client->post($this->gatewayUrl, [
                'json' => ['mode' => 'GET_STATE'],
            ]);
            
            $result = json_decode($response->getBody()->getContents());
            
            return response()->json(['status' => 'success', 'data' => $result], 200);
        } catch (RequestException $e) {
            return response()->json(['status' => 'error', 'message' => 'Could not retrieve status from Node.js Gateway.'], 503);
        }
    }
}
