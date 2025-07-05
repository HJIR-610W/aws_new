#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os
import chardet
import shutil
from pathlib import Path

def detect_encoding(file_path):
    """파일의 인코딩을 감지합니다."""
    try:
        with open(file_path, 'rb') as file:
            raw_data = file.read()
            result = chardet.detect(raw_data)
            return result['encoding'] if result['confidence'] > 0.7 else None
    except Exception as e:
        print(f"인코딩 감지 실패 - {file_path}: {e}")
        return None

def convert_to_euckr(file_path, original_encoding):
    """파일을 EUC-KR로 변환합니다."""
    try:
        # 백업 파일 생성
        backup_path = f"{file_path}.backup"
        shutil.copy2(file_path, backup_path)
        
        # 원본 인코딩으로 읽기
        with open(file_path, 'r', encoding=original_encoding) as file:
            content = file.read()
        
        # EUC-KR로 저장 (에러 처리 포함)
        with open(file_path, 'w', encoding='euc-kr', errors='replace') as file:
            file.write(content)
        
        # 백업 파일 삭제
        os.remove(backup_path)
        return True
        
    except UnicodeEncodeError as e:
        print(f"EUC-KR 변환 불가 (지원하지 않는 문자 포함) - {file_path}: {e}")
        # 실패 시 백업에서 복원
        if os.path.exists(backup_path):
            shutil.move(backup_path, file_path)
        return False
    except Exception as e:
        print(f"변환 실패 - {file_path}: {e}")
        # 실패 시 백업에서 복원
        if os.path.exists(backup_path):
            shutil.move(backup_path, file_path)
        return False

def should_process_file(file_path):
    """처리할 파일인지 확인합니다."""
    # 텍스트 파일 확장자 목록
    text_extensions = {
        '.txt', '.py', '.c', '.cpp', '.h', '.hpp', '.java', '.js', '.html', 
        '.htm', '.css', '.xml', '.json', '.csv', '.sql', '.md', '.rst',
        '.ini', '.cfg', '.conf', '.log', '.bat', '.sh', '.ps1'
    }
    
    file_extension = Path(file_path).suffix.lower()
    
    # 확장자가 없는 경우도 처리 (README, Makefile 등)
    if not file_extension:
        filename = Path(file_path).name.lower()
        text_files = {'readme', 'makefile', 'dockerfile', 'license', 'changelog'}
        return filename in text_files
    
    return file_extension in text_extensions

def convert_directory_to_euckr(directory_path='.'):
    """디렉토리의 모든 파일을 EUC-KR로 변환합니다."""
    directory_path = Path(directory_path)
    
    if not directory_path.exists():
        print(f"디렉토리가 존재하지 않습니다: {directory_path}")
        return
    
    print(f"EUC-KR 변환 시작: {directory_path.absolute()}")
    print("-" * 50)
    
    total_files = 0
    converted_files = 0
    already_euckr_files = 0
    failed_files = 0
    
    # 모든 파일을 재귀적으로 탐색
    for file_path in directory_path.rglob('*'):
        if file_path.is_file() and should_process_file(file_path):
            total_files += 1
            
            # 현재 인코딩 감지
            current_encoding = detect_encoding(file_path)
            
            if current_encoding is None:
                print(f"건너뜀 (인코딩 감지 실패): {file_path}")
                failed_files += 1
                continue
            
            # 이미 EUC-KR인 경우
            if current_encoding.lower() in ['euc-kr', 'cp949']:
                print(f"이미 EUC-KR: {file_path}")
                already_euckr_files += 1
                continue
            
            # EUC-KR로 변환
            print(f"변환 중 ({current_encoding} → EUC-KR): {file_path}")
            if convert_to_euckr(file_path, current_encoding):
                converted_files += 1
                print(f"? 변환 완료: {file_path}")
            else:
                failed_files += 1
    
    # 결과 요약
    print("-" * 50)
    print("변환 완료!")
    print(f"총 처리된 파일: {total_files}")
    print(f"변환된 파일: {converted_files}")
    print(f"이미 EUC-KR인 파일: {already_euckr_files}")
    print(f"실패한 파일: {failed_files}")
    
    if failed_files > 0:
        print("\n※ 주의: EUC-KR로 표현할 수 없는 특수 문자가 포함된 파일은 변환에 실패할 수 있습니다.")

def main():
    """메인 함수"""
    print("파일 인코딩 EUC-KR 변환기")
    print("=" * 50)
    print("※ 경고: EUC-KR은 한글과 기본 ASCII 문자만 지원합니다.")
    print("※ 다른 언어의 특수문자는 손실될 수 있습니다.")
    print()
    
    # 사용자 입력 받기
    target_dir = input("변환할 디렉토리 경로를 입력하세요 (엔터 시 현재 디렉토리): ").strip()
    if not target_dir:
        target_dir = '.'
    
    # 확인 메시지
    confirm = input(f"\n'{os.path.abspath(target_dir)}' 디렉토리의 모든 파일을 EUC-KR로 변환하시겠습니까? (y/N): ")
    if confirm.lower() != 'y':
        print("취소되었습니다.")
        return
    
    # 변환 실행
    convert_directory_to_euckr(target_dir)

if __name__ == "__main__":
    # chardet 라이브러리 설치 확인
    try:
        import chardet
    except ImportError:
        print("chardet 라이브러리가 필요합니다.")
        print("다음 명령어로 설치하세요: pip install chardet")
        exit(1)
    
    main()