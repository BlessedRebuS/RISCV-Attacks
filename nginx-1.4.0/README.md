nginx-1.4.0
===========

For the [analysis of CVE-2013-2028](https://www.vnsecurity.net/research/2013/05/21/analysis-of-nginx-cve-2013-2028.html)

## Compiling for RISC-V

There is a custom _install_ directory inside the nginx folder with the configuration required to compile nginx on RISCV.

In the root directory configure the environment in this way

```bash
./configure \
    --prefix=<PATH>/install \
    --sbin-path=<PATH>/install \
    --conf-path=<PATH>/install/nginx.conf \
    --pid-path=<PATH>/install/nginx.pid \
    --error-log-path=<PATH>/install/error.log \
    --without-http_rewrite_module
```

Then do 

```bash
make
```

This will build the program and use the config found in the _install_ folder. If the config is missing it will create it. By default I am choosing the port **1111** in order to run the binary without root permission.

## Compiled binary\
The **nginx** binary for RISC-V that I compiled is available [here](https://github.com/BlessedRebuS/RISCV-Attacks/blob/main/nginx-1.4.0/objs/nginx). Make sure to follow the project structure to run the binary.

## Run the binary
To run the binary

```bash
./nginx
```

Testing with

```bash
curl localhost 1111
```

If all is working It should say the following

```html
<!DOCTYPE html>
<html>
<body>
  <h1>My First Heading</h1>
  <p>My first paragraph.</p>
</body>
</html>
```