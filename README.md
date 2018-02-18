<h1 align="center"> ASSIGNMENT OS:1 Basic Shell </h1>

## MY SHELL COMMAND

### 1. Interactive

การทํางานแบบ **interactive** ระบบจะแสดง **mysh>** รอให้ผู้ใช้ป้อมคำสั่ง `command`

>**mysh>** `command`

```
mysh> ls
```
>**mysh>** `command` _`argument..`_

* *สามารถใส่ _`argument`_ ได้มากกว่า 1 _`argument`_*

```
mysh> ls -l
mysh> ls -l -a
```
>**mysh>** `command` _`argument`_ ;  `command` _`argument`_ ; `...`

* *ในแต่ละบรรทัดของ **mysh** สามารถใส่ได้มากกว่า 1 `command` โดยแต่ละ `command` จะคั่นด้วย **`;`***
* *คําสั่ง `command` เหล่านี้ จะถูกเรียกขึ้นมาพร้อมกัน **(concurrent)** และระบบจะแสดง  **mysh** เมื่อทุกคําสั่งทํางานเสร็จหมดแล้ว*

```
mysh> ls ; cat file
mysh> ls -l ; cat file1
mysh> ls -l ; cat file1 ; greb foo file2
```
##

### 2. Batch file

การทํางานแบบ **Batch file** ระบบจะอ่าน `command` จากไฟล์ และจะทํางานทีละคำสั่งจนเสร็จ โดยไม่แสดง **prompt**

>**Terminal>** `program` `batch file`
>
```
Terminal>./mysh file.bat
```

##

### 3. Built-In command
คำสั่ง `command` อื่นๆ ที่สร้างโดยผู้เขียน


>**mysh>** `command`

* *ปิดโปรแกรม*

```
mysh> quit
```

##