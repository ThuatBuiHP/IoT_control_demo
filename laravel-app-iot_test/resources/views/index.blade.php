<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Tạo Bài Viết</title>
    <!-- Bootstrap 5 CDN -->
    <link href="https://cdn.jsdelivr.net/npm/bootstrap@5.3.3/dist/css/bootstrap.min.css" rel="stylesheet">
</head>
<body>
<div class="container mt-5">
    <form id="postForm">
        <div class="mb-3">
            <label for="title" class="form-label">Tiêu đề</label>
            <input type="text" class="form-control" id="title" name="title" required>
        </div>
        <div class="mb-3">
            <label for="content" class="form-label">Nội dung</label>
            <textarea class="form-control" id="content" name="content" rows="5" required></textarea>
        </div>
        <button type="submit" class="btn btn-primary">Tạo Bài Viết</button>
    </form>
</div>

<!-- Bootstrap JS -->
<script src="https://cdn.jsdelivr.net/npm/bootstrap@5.3.3/dist/js/bootstrap.bundle.min.js"></script>
<script>
document.getElementById("postForm").addEventListener("submit", async function(e) {
    e.preventDefault(); // chặn submit mặc định

    // Gom dữ liệu form thành object
    const postData = {
        title: document.getElementById("title").value,
        content: document.getElementById("content").value
    };

    try {
        const response = await fetch("/new-posts", { // thay "/api/posts" bằng route backend của bạn
            method: "POST",
            headers: {
                "Content-Type": "application/json",
                "X-CSRF-TOKEN": "{{ csrf_token() }}" // nếu bạn dùng Laravel
            },
            body: JSON.stringify(postData)
        });

        const result = await response.json();
        console.log("Kết quả:", result);
        alert("Tạo bài viết thành công!");
    } catch (error) {
        console.error("Lỗi:", error);
        alert("Có lỗi xảy ra khi tạo bài viết.");
    }
});
</script>
</body>
</html>