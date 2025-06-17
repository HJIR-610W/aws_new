# -*- coding: euc-kr -*-

import os
import sys

# 현재 스크립트 위치 기준
base_path = os.path.dirname(os.path.abspath(__file__))

# 제외할 확장자
exclude_extensions = ['.py','.git','.gitignore']

# 인코딩 변환할 함수
def convert_encoding(file_path):
  try:
    with open(file_path, 'r', encoding='euc-kr') as f:
      content = f.read()
    with open(file_path, 'w', encoding='utf-8') as f:
      f.write(content)
    print(f'[변환 성공] {file_path}')
  except Exception as e:
    print(f'[변환 실패] {file_path} : {e}')

# 재귀적으로 모든 파일 탐색
for root, dirs, files in os.walk(base_path):
  for file in files:
    file_path = os.path.join(root, file)
    # 자기 자신(.py) 파일 제외
    if file_path == os.path.abspath(__file__):
      continue
    # 제외 확장자
    if os.path.splitext(file)[1].lower() in exclude_extensions:
      continue
    # 바이너리 파일은 건너뜀
    try:
      convert_encoding(file_path)
    except Exception as e:
      print(f'[건너뜀] {file_path} : {e}')
