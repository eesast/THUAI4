## docker打包compiler

compiler 镜像的运行需要进行挂载盘的绑定

```shell
docker run --name=asd -v  ~/mnt:/usr/local/mnt --rm   mrasamu/thuai_compiler: xx版本
```

