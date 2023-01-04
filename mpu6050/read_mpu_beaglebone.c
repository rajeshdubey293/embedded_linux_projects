#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <stdint.h>

/* Address of the MP6050 on the BeagleBone Black board */
#define I2C_ADDRESS 0x68

// MPU6050 structure
typedef struct
{

    int16_t Accel_X_RAW;
    int16_t Accel_Y_RAW;
    int16_t Accel_Z_RAW;
    double Ax;
    double Ay;
    double Az;

    int16_t Gyro_X_RAW;
    int16_t Gyro_Y_RAW;
    int16_t Gyro_Z_RAW;
    double Gx;
    double Gy;
    double Gz;

    float Temperature;
} MPU6050_t;

MPU6050_t MPU_Data = {0};
const double Accel_Z_corrector = 14418.0;

void Display_MPU6050_Data(MPU6050_t *MPU_DataStruct);

int main(void)
{
    int fd;               // file descriptor
    int status = 0;       // for writing and reading status
    char txBuf[2] = {0};  // for writing
    char rxBuf[14] = {0}; // for reading
    int16_t temp = 0;

    /* Open the adapter and set the address of the I2C device */
    fd = open("/dev/i2c-2", O_RDWR);
    if (fd < 0)
    {
        perror("/dev/i2c-2:");
        return 1;
    }

    /* Set the address of the i2c slave device */
    if (ioctl(fd, I2C_SLAVE, I2C_ADDRESS) == -1)
    {
        perror("ioctl I2C_SLAVE");
        return 1;
    }

    /* write 1  bytes at 0x6B, to wake up mpu6050*/
    txBuf[0] = 0x6B;
    txBuf[1] = 0x00;

    status = write(fd, txBuf, 2);
    if (status == -1)
    {
        perror("write");
        return 1;
    }
    for (;;)
    {
        /* Set the 8-bit address to read */
        txBuf[0] = 0x3B; /* address byte*/
        status = write(fd, txBuf, 1);
        if (status == -1)
        {
            perror("write");
            return 1;
        }
        /* Now read 14 bytes from that address */
        status = read(fd, rxBuf, 14);
        if (status == -1)
        {
            perror("read");
            return 1;
        }
        MPU_Data.Accel_X_RAW = (int16_t)(rxBuf[0] << 8 | rxBuf[1]);
        MPU_Data.Accel_Y_RAW = (int16_t)(rxBuf[2] << 8 | rxBuf[3]);
        MPU_Data.Accel_Z_RAW = (int16_t)(rxBuf[4] << 8 | rxBuf[5]);
        temp = (int16_t)(rxBuf[6] << 8 | rxBuf[7]);
        MPU_Data.Gyro_X_RAW = (int16_t)(rxBuf[8] << 8 | rxBuf[9]);
        MPU_Data.Gyro_Y_RAW = (int16_t)(rxBuf[10] << 8 | rxBuf[11]);
        MPU_Data.Gyro_Z_RAW = (int16_t)(rxBuf[12] << 8 | rxBuf[13]);

        MPU_Data.Ax = MPU_Data.Accel_X_RAW / 16384.0;
        MPU_Data.Ay = MPU_Data.Accel_Y_RAW / 16384.0;
        MPU_Data.Az = MPU_Data.Accel_Z_RAW / Accel_Z_corrector;
        MPU_Data.Temperature = (float)((int16_t)temp / (float)340.0 + (float)36.53);
        MPU_Data.Gx = MPU_Data.Gyro_X_RAW / 131.0;
        MPU_Data.Gy = MPU_Data.Gyro_Y_RAW / 131.0;
        MPU_Data.Gz = MPU_Data.Gyro_Z_RAW / 131.0;
        Display_MPU6050_Data(&MPU_Data);
        sleep(0.05);
    }
    close(fd);
    return 0;
}
void Display_MPU6050_Data(MPU6050_t *MPU_DataStruct)
{
    char buffer[100] = {0};
    sprintf(buffer, "Ax = %.2lf\tAy = %.2lf\tAz = %.2lf\tGx = %.2lf\tGy = %.2lf\tGz = %.2lf\tTemp = %.2f\r\n",
            MPU_DataStruct->Ax, MPU_DataStruct->Ay, MPU_DataStruct->Az,
            MPU_DataStruct->Gx, MPU_DataStruct->Gy, MPU_DataStruct->Gz,
            MPU_DataStruct->Temperature);
    printf("%s", buffer);
}
