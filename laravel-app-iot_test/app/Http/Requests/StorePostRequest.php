<?php

namespace App\Http\Requests;

use Illuminate\Foundation\Http\FormRequest;


class StorePostRequest extends FormRequest
{
    public function authorize(): bool
    {
        return true; // cho phép tất cả request, có thể chỉnh theo nhu cầu
    }

    public function rules(): array
    {
        return [
            'title'   => 'required|string|max:255',
            'content' => 'required|string',
        ];
    }
}

