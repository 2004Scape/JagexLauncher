/*
 * Copyright 2010 Jagex Limited.
 *
 * This file is part of JagexLauncher.
 *
 * JagexLauncher is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 only, as
 * published by the Free Software Foundation.
 *
 * JagexLauncher is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * version 2 for more details (a copy is included in the LICENSE file that
 * accompanied this code).
 *
 * You should have received a copy of the GNU General Public License version
 * 2 along with this work; if not, see <http://www.gnu.org/licenses/>
 *
 */

#include "stdafx.h"
#include <iostream>

using std::cout;
using std::endl;

#define MAX_LINE_LENGTH 1024

int mainprogram(char* game) {
	FILE* f;
	char** paramlines;
	int l_paramlines = 0;
	char prmfile[260];

	sprintf_s(prmfile, 260, "../%s/%s.prm", game, game);
	if (fopen_s(&f, prmfile, "r") != 0) {
		MessageBox(NULL, TEXT("Unable to load parameter file. Please reinstall the program."), TEXT("Error"), MB_OK);
		return 1;
	} else {
		char line[MAX_LINE_LENGTH];
		while (fgets(line, MAX_LINE_LENGTH, f) != NULL) {
			l_paramlines++;
		}

		paramlines = new char* [l_paramlines];
		fseek(f, 0, SEEK_SET);

		int i = 0;
		while (fgets(line, MAX_LINE_LENGTH, f) != NULL) {
			int len = strlen(line);
			for (int j = len - 1; j >= 0; j--) {
				if (line[j] != '\n' && line[j] != '\r') break;
				line[j] = 0;
			}

			paramlines[i] = new char[len + 1];
			strcpy_s(paramlines[i], len + 1, line);
			i++;
		}

	}

	if (l_paramlines < 1) {
		MessageBox(NULL, TEXT("Empty parameter file. Please reinstall the program."), TEXT("Error"), MB_OK);
		return 2;
	}

	JavaVM* jvm = NULL;
	JNIEnv* env = NULL;
	if (LoadLibrary(TEXT("client/jvm.dll")) == NULL) {
		return 3;
	}

	JavaVMInitArgs vm_args{};
	vm_args.version = JNI_VERSION_1_8;
	vm_args.nOptions = l_paramlines - 1;
	vm_args.options = new JavaVMOption[l_paramlines - 1];
	for (int i = 0; i < l_paramlines - 1; i++) {
		vm_args.options[i].optionString = paramlines[i];
	}
	vm_args.ignoreUnrecognized = false;

	// debugging mode exception: https://stackoverflow.com/a/36258856
	// SEGV (or exception 0xC0000005) is also generated intentionally on JVM startup to verify certain CPU/OS features
	jint result = JNI_CreateJavaVM(&jvm, reinterpret_cast<void**>(&env), &vm_args);
	if (result != JNI_OK) {
		return 0;
	}

	jclass cls = env->FindClass(paramlines[l_paramlines - 1]);
	if (cls != NULL) {
		jmethodID mid = env->GetStaticMethodID(cls, "main", "([Ljava/lang/String;)V");
		jobjectArray args = env->NewObjectArray(1, env->FindClass("java/lang/String"), NULL);
		env->SetObjectArrayElement(args, 0, env->NewStringUTF(game));
		env->CallStaticVoidMethod(cls, mid, args);
	}

	jvm->DestroyJavaVM();
	return 0;
}

#ifdef _CONSOLE
int _tmain(int argc, TCHAR** argv) {
	if (argc < 2) return 0;
#ifdef UNICODE
	int len = _tcslen(argv[1]);
	char* game = new char[len + 1];
	WideCharToMultiByte(CP_ACP, 0, argv[1], len + 1, game, len + 1, NULL, NULL);
	int ret = mainprogram(game);
	delete[] game;
	return ret;
#else
	return mainprogram(argv[0]);
#endif
}
#else
int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nShowCmd) {
	return mainprogram(lpCmdLine);
}
#endif