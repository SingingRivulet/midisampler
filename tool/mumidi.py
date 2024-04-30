import os
import subprocess

def process_mid_files(source_dir, dest_dir):
    # 遍历源目录
    for root, dirs, files in os.walk(source_dir):
        # 在目标目录中创建相同的子目录
        for dir_name in dirs:
            source_sub_dir = os.path.join(root, dir_name)
            dest_sub_dir = source_sub_dir.replace(source_dir, dest_dir)
            if not os.path.exists(dest_sub_dir):
                os.makedirs(dest_sub_dir)

        # 复制.mid文件到目标目录
        for file in files:
            if file.lower().endswith(".mid"):
                source_file = os.path.join(root, file)
                dest_file = source_file.replace(source_dir, dest_dir)+".mumidi"
                if os.path.exists(dest_file):
                    continue
                # shutil.copyfile(source_file, dest_file)
                print(f"process {source_file} to {dest_file}")
                subprocess.run(["./mumidiEncoder" , "-1" , "0" , source_file, dest_file])
                print("success")

# 调用函数并指定源目录和目标目录
source_directory = "/nfs/midi_45w/midi"  # 替换为源目录的路径
destination_directory = "/data/midi_45w/mumidi"  # 替换为目标目录的路径

process_mid_files(source_directory, destination_directory)
