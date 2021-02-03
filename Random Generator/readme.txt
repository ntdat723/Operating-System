Mục đích: Tạo một dãy số ngẫu nhiên, chạy liên tục cho tới khi user dừng bằng Ctrl-C.

Source Code:
static ssize_t random_read(struct file *f, char __user *buf, size_t len, loff_t *off)
{
	char afterCast;											//a variable of type char to hold the value of the random-generated number
	get_random_bytes(&randomNumber, 1);								//get_random_bytes() returns the requested number of random bytes and stores them in the buffer
	while (randomNumber < 0 || randomNumber >= 10)							//while loop to check if the random-generated value is in range from 0 to 9
		get_random_bytes(&randomNumber, 1);								//if not, keep performing get_random_bytes() until it is
	afterCast = randomNumber + '0';									//Casting an integer to char type
	if (copy_to_user(buf, &afterCast, 1))								//copy_to_user() copies data from kernel space to user space. Return 0 on success
		return -EFAULT;
	else
		return 1;
}

Ý tưởng: Hàm get_random_bytes() được sử dụng để tạo ra các số ngẫu nhiên. Sau đó sẽ được kiểm tra xem có nằm trong khoảng từ 0 đến 9 hay không, nếu không thì chạy lại hàm get_random_bytes()
	cho đến khi thoả mãn điều kiện. Sau khi có được số mong muốn, chương trình sẽ cast số ngẫu nhiên đó thành dạng char để copy ra user space.
