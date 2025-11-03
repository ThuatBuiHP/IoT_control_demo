
<?php

use Illuminate\Support\Facades\Route;
use App\Http\Controllers\PostController;

Route::get('/', function () {
    return view('index');
});

Route::post('/new-posts', [PostController::class, 'store']);

