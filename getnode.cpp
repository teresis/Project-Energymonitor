#include "getnode.h"
#include <QFile>
#include <sys/ioctl.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <QDebug>
#include <QProcess>
#include <QSysInfo>

GetNode::GetNode()
{
   // usage[8] = {0,};

    for (int i = 0; i < 8; i++) {
        QString temp;
        temp.sprintf("/sys/devices/system/cpu/cpu%d/cpufreq/cpuinfo_cur_freq", i);
        cpu_node_list[i] = temp;
    }
}

void GetNode::GetSystemInfo()
{
    QProcess p1;
    QString cmd;

    cmd = "lsb_release -r | awk '{print $2}'";
    p1.start("bash", QStringList() << "-c" << cmd);
    p1.waitForFinished(-1);
    os_ver = p1.readAllStandardOutput().simplified();

    cmd = "lsb_release -i | awk '{print $3}'";
    p1.start("bash", QStringList() << "-c" << cmd);
    p1.waitForFinished(-1);
    os_name = p1.readAllStandardOutput().simplified();

    cmd = "uname -r";
    p1.start("bash", QStringList() << "-c" << cmd);
    p1.waitForFinished(-1);
    kernel_ver = p1.readAllStandardOutput().simplified();
}

QString GetNode::GetGPUCurFreq()
{

    QFile *fp;
    QString freq;

    if (kernel_ver[0] == '4') {
        //freq.sprintf("%d", 600);
	fp = new QFile(GPUFREQ_NODE_v4);
        if (!fp->open(QIODevice::ReadOnly))
            return 0;
        freq = fp->readLine();
	gpufreq = freq.toInt()/1000000;
        freq.sprintf("%d", gpufreq);
        fp->close();
        delete fp;
    } else {
        fp = new QFile(GPUFREQ_NODE);
        if (!fp->open(QIODevice::ReadOnly))
            return 0;
        freq = fp->readLine();
        freq.sprintf("%d", freq.toInt()/1000000);
        fp->close();
        delete fp;
    }

    return freq;
}

QString GetNode::GetCPUCurFreq(int cpuNum)
{
    QFile *fp = new QFile(cpu_node_list[cpuNum]);
    QString freq;

    if (!fp->open(QIODevice::ReadOnly))
        return 0;

    freq = fp->readLine();
    freq.sprintf("%d", freq.toInt()/1000);

    fp->close();

    delete fp;
    return freq;
}

QString GetNode::GetCPUTemp(int cpuNum)
{
    QFile *fp;

    char buf[16];

    if (kernel_ver[0] == '4') {
        fp = new QFile(TEMP_NODE_v4 + QString::number(cpuNum) + "/temp");

        if (!fp->open(QIODevice::ReadOnly)) {
            return 0;
        }

        fp->read(buf, 2);
        buf[2] = '\0';
        fp->close();
        delete fp;

        return buf;
    } else {
        fp = new QFile(TEMP_NODE);

        if (!fp->open(QIODevice::ReadOnly)) {
            return 0;
        }

        for (int i = 0; i < cpuNum + 1; i++)
            fp->read(buf, 16);
        fp->close();
        delete fp;
        buf[12] = '\0';

        return &buf[10];
    }
}

/*
int GetNode::open_sensor(const char *node, sensor_t *sensor)
{
    if ((sensor->fd = open(node, O_RDWR)) < 0)
        qDebug() << node << "Open Fail";

    return sensor->fd;
}

int GetNode::OpenINA231()
{
    if (open_sensor(DEV_SENSOR_ARM, &sensor[SENSOR_ARM]) < 0)
        return -1;
    if (open_sensor(DEV_SENSOR_MEM, &sensor[SENSOR_MEM]) < 0)
        return -1;
    if (open_sensor(DEV_SENSOR_KFC, &sensor[SENSOR_KFC]) < 0)
        return -1;
    if (open_sensor(DEV_SENSOR_G3D, &sensor[SENSOR_G3D]) < 0)
        return -1;

    if (read_sensor_status(&sensor[SENSOR_ARM]))
        return -1;
    if (read_sensor_status(&sensor[SENSOR_MEM]))
        return -1;
    if (read_sensor_status(&sensor[SENSOR_KFC]))
        return -1;
    if (read_sensor_status(&sensor[SENSOR_G3D]))
        return -1;

    if (!sensor[SENSOR_ARM].data.enable)
        enable_sensor(&sensor[SENSOR_ARM], 1);
    if (!sensor[SENSOR_MEM].data.enable)
        enable_sensor(&sensor[SENSOR_MEM], 1);
    if (!sensor[SENSOR_KFC].data.enable)
        enable_sensor(&sensor[SENSOR_KFC], 1);
    if (!sensor[SENSOR_G3D].data.enable)
        enable_sensor(&sensor[SENSOR_G3D], 1);

    return 0;
}

void GetNode::CloseINA231()
{
    if (sensor[SENSOR_ARM].data.enable)
        enable_sensor(&sensor[SENSOR_ARM], 0);
    if (sensor[SENSOR_MEM].data.enable)
        enable_sensor(&sensor[SENSOR_MEM], 0);
    if (sensor[SENSOR_KFC].data.enable)
        enable_sensor(&sensor[SENSOR_KFC], 0);
    if (sensor[SENSOR_G3D].data.enable)
        enable_sensor(&sensor[SENSOR_G3D], 0);

    close_sensor(&sensor[SENSOR_ARM]);
    close_sensor(&sensor[SENSOR_MEM]);
    close_sensor(&sensor[SENSOR_KFC]);
    close_sensor(&sensor[SENSOR_G3D]);
}

void GetNode::GetINA231()
{
    read_sensor(&sensor[SENSOR_ARM]);
    read_sensor(&sensor[SENSOR_MEM]);
    read_sensor(&sensor[SENSOR_KFC]);
    read_sensor(&sensor[SENSOR_G3D]);

    armuV = (float)(sensor[SENSOR_ARM].data.cur_uV / 100000) / 10;
    armuA = (float)(sensor[SENSOR_ARM].data.cur_uA / 1000) / 1000;
    armuW = (float)(sensor[SENSOR_ARM].data.cur_uW / 1000) / 1000;

    memuV = (float)(sensor[SENSOR_MEM].data.cur_uV / 100000) / 10;
    memuA = (float)(sensor[SENSOR_MEM].data.cur_uA / 1000) / 1000;
    memuW = (float)(sensor[SENSOR_MEM].data.cur_uW / 1000) / 1000;

    kfcuV = (float)(sensor[SENSOR_KFC].data.cur_uV / 100000) / 10;
    kfcuA = (float)(sensor[SENSOR_KFC].data.cur_uA / 1000) / 1000;
    kfcuW = (float)(sensor[SENSOR_KFC].data.cur_uW / 1000) / 1000;

    g3duV = (float)(sensor[SENSOR_G3D].data.cur_uV / 100000) / 10;
    g3duA = (float)(sensor[SENSOR_G3D].data.cur_uA / 1000) / 1000;
    g3duW = (float)(sensor[SENSOR_G3D].data.cur_uW / 1000) / 1000;

}

void GetNode::enable_sensor(sensor_t *sensor, unsigned char enable)
{
    if (sensor->fd > 0) {
        sensor->data.enable = enable ? 1 : 0;
        if (ioctl(sensor->fd, INA231_IOCSSTATUS, &sensor->data) < 0)
            qDebug() << "IOCTL Error";
    }
}

int GetNode::read_sensor_status(sensor_t *sensor)
{
    if (sensor->fd > 0) {
        if (ioctl(sensor->fd, INA231_IOCGSTATUS, &sensor->data) < 0)
            qDebug() << sensor->data.name << "IOCTL Error";
    }
    return 0;
}

void GetNode::read_sensor(sensor_t *sensor)
{
    if (sensor->fd > 0) {
        if (ioctl(sensor->fd, INA231_IOCGREG, &sensor->data) < 0)
            qDebug() << sensor->data.name << "IOCTL Error!";
    }
}

void GetNode::close_sensor(sensor_t *sensor)
{
    if (sensor->fd > 0)
        close(sensor->fd);
}

int GetNode::calUsage(int cpu_idx, int user, int nice, int system, int idle)
{
    long total = 0;
    long usage = 0;
    int diff_user, diff_system, diff_idle;

    diff_user = mOldUserCPU[cpu_idx] - user;
    diff_system = mOldSystemCPU[cpu_idx] - system;
    diff_idle = mOldIdleCPU[cpu_idx] - idle;

    total = diff_user + diff_system + diff_idle;
    if (total != 0)
        usage = diff_user * 100 / total;

    mOldUserCPU[cpu_idx] =user;
    mOldSystemCPU[cpu_idx] = system;
    mOldIdleCPU[cpu_idx] = idle;

    return usage;
}

int GetNode::GetCPUUsage(void)
{
    char buf[80] = {0,};
    char cpuid[8] = "cpu";
    int findCPU = 0;
    int user, system, nice, idle;
    FILE *fp;
    int cpu_index = 0;

    fp = fopen("/proc/stat", "r");
    if (fp == NULL)
        return 0;

    while (fgets(buf, 80, fp)) {
        char temp[4] = "cpu";
        temp[3] = '0' + cpu_index;
        if (!strncmp(buf, temp, 4)) {
            findCPU = 1;
            sscanf(buf, "%s %d %d %d %d",
                   cpuid, &user, &nice, &system, &idle);
            usage[cpu_index] = calUsage(cpu_index, user, nice, system, idle);
            cpu_index++;
        }
        if (!strncmp(buf, "intr", 4))
            break;
        if (findCPU == 0)
            mOldUserCPU[cpu_index] = mOldSystemCPU[cpu_index] = mOldIdleCPU[cpu_index] = 0;
        else
            findCPU = 0;
    }

    fclose(fp);

    return 0;
}
*/
