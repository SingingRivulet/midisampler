# midi数据集采样工具
## 使用方法：
### 编译：
下载本项目，然后make即可
### 使用：
`./midi-samlper 输入midi文件 输出文本文件 旋律音轨 和弦音轨`
如
`./midi-samlper 1.mid 1.txt 1 2`  
批量处理：`find ./MIDI/ -name *.mid -exec ./midi-sampler {} sampled.txt 0 1 \;`  
### 和弦生成工具使用：  
先执行父目录的server.py，然后进入tool目录，执行./chordgentext
