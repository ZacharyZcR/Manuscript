<?php
class User {
    public $name;
    public $isAdmin;

    public function __construct($name, $isAdmin) {
        $this->name = $name;
        $this->isAdmin = $isAdmin;
    }

    public function __wakeup() {
        if ($this->isAdmin) {
            echo "Admin access granted for user: {$this->name}\n";
        }
    }
}

// 模拟不安全的反序列化
if (isset($_GET['data'])) {
    $data = $_GET['data'];
    $object = unserialize($data);
    var_dump($object);
}
?>
