pipeline {
    agent any
    stages {
        stage('Git Prep') {
            steps {
                checkout poll: false, scm: scmGit(branches: [[name: '*/develop']], userRemoteConfigs: [[url: 'https://github.com/smittytone/Depot.git']])
                checkout poll: false, scm: scmGit(branches: [[name: '*/master']], extensions: [submodule(recursiveSubmodules: true, reference: '')], userRemoteConfigs: [[url: 'https://github.com/raspberrypi/pico-sdk.git']])
                sh 'ls $PWD'
            }
        }
        stage('Linux Test Build') {
            steps {
                cmakeBuild buildDir: 'linux/build', cleanBuild: true, installation: 'InSearchPath', sourceDir: 'linux', steps: [[withCmake: true]]
            }
        }
        stage('Firmware Test Build') {
            steps {
                cmakeBuild buildDir: 'build', cleanBuild: true, installation: 'InSearchPath', steps: [[withCmake: true]]
            }
        }
    }
}
