ftp需预先增加用户。

在调用ftp6FileSend的时候，由于版本用户名写入了nodeb_v3，所以每次在osp的服务器端调用此函数的时候，一定要在osp的FTP server那里配置上用户名为nodeb_v3，密码为nodeb_v3的账户。