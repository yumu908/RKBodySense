package com.rockchips.bodysense;

/**
 * A 2D Kalman filter for smoothing noisy (x, y) position measurements.
 * State vector: [x, vx, y, vy] - constant velocity model.
 * This filter fuses noisy sensor data with a motion model to produce a smoother,
 * more accurate estimate of position and velocity.
 */
public class KalmanFilter2D {

    private final double dt; // time step between updates

    // State: [positionX, velocityX, positionY, velocityY]
    private double[] state;

    // Estimate error covariance matrix
    private double[][] covariance;

    // State transition model
    private double[][] transition;

    // Observation (measurement) model
    private double[][] observation;

    // Process noise covariance
    private double[][] processNoise;

    // Measurement noise covariance
    private double[][] measurementNoise;

    public long lastUpdateTime;     // 最后一次成功更新的时间

    /**
     * Creates a Kalman filter for 2D coordinates with given time step.
     *
     * @param timeStep seconds between updates (e.g., 0.1 for 10 Hz)
     */
    public KalmanFilter2D(double timeStep) {
        this.dt = timeStep;
        initializeState();
        initializeCovariance();
        initializeModels();
        initializeNoise();
        this.lastUpdateTime = System.currentTimeMillis();
    }

    private void initializeState() {
        state = new double[]{0, 0, 0, 0}; // x, vx, y, vy
    }

    private void initializeCovariance() {
        covariance = new double[][]{
                {1000, 0,    0,    0   },
                {0,    1000, 0,    0   },
                {0,    0,    1000, 0   },
                {0,    0,    0,    1000}
        };
    }

    private void initializeModels() {
        // x_{k} = F x_{k-1}
        transition = new double[][]{
                {1, dt, 0,  0 },
                {0, 1,  0,  0 },
                {0, 0,  1, dt},
                {0, 0,  0, 1 }
        };

        // z = H x (we only measure position)
        observation = new double[][]{
                {1, 0, 0, 0},
                {0, 0, 1, 0}
        };
    }

    private void initializeNoise() {
        double q = 0.01; // process noise intensity
        processNoise = new double[][]{
                {q, 0, 0, 0},
                {0, q, 0, 0},
                {0, 0, q, 0},
                {0, 0, 0, q}
        };

        double r = 0.1; // measurement noise variance
        measurementNoise = new double[][]{
                {r, 0},
                {0, r}
        };
    }

    /**
     * Predicts the next state based on the motion model.
     */
    public void predict() {
        // state = transition * state
        double[] newState = new double[4];
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                newState[i] += transition[i][j] * state[j];
            }
        }
        state = newState;

        // covariance = transition * covariance * transpose(transition) + processNoise
        double[][] temp1 = multiply(covariance, transpose(transition));
        double[][] temp2 = multiply(transition, temp1);
        covariance = add(temp2, processNoise);
    }

    /**
     * Updates the state using a new measurement [x, y].
     *
     * @param measurement array of {x, y} observed coordinates
     */
    public void update(double[] measurement) {
        // innovation: y = z - H * x
        double[] innovation = new double[2];
        innovation[0] = measurement[0] - (observation[0][0] * state[0] + observation[0][2] * state[2]);
        innovation[1] = measurement[1] - (observation[1][0] * state[0] + observation[1][2] * state[2]);

        // innovation covariance: S = H * P * H^T + measurementNoise
        double[][] s1 = multiply(covariance, transpose(observation));
        double[][] s2 = multiply(observation, s1);
        double[][] innovationCovariance = add(s2, measurementNoise);

        // Kalman gain: K = P * H^T * S^{-1}
        double[][] kalmanGainPre = multiply(covariance, transpose(observation));
        double[][] innovationInverse = inverse2x2(innovationCovariance);
        double[][] kalmanGain = multiply(kalmanGainPre, innovationInverse);

        // state = state + K * innovation
        for (int i = 0; i < 4; i++) {
            state[i] += kalmanGain[i][0] * innovation[0] + kalmanGain[i][1] * innovation[1];
        }

        // covariance = (I - K * H) * P
        double[][] identity = {
                {1, 0, 0, 0},
                {0, 1, 0, 0},
                {0, 0, 1, 0},
                {0, 0, 0, 1}
        };
        double[][] kh = multiply(kalmanGain, observation);
        double[][] updatedCovariance = multiply(subtract(identity, kh), covariance);
        covariance = updatedCovariance;

        // reset miss-count
        this.lastUpdateTime = System.currentTimeMillis();
    }

    /**
     * Gets the current estimated position.
     *
     * @return array of {x, y}
     */
    public double[] getPosition() {
        return new double[]{state[0], state[2]};
    }

    /**
     * Gets the current estimated velocity.
     *
     * @return array of {vx, vy}
     */
    public double[] getVelocity() {
        return new double[]{state[1], state[3]};
    }

    /**
     * Resets the filter to initial state (useful for new tracking session).
     */
    public void reset() {
        initializeState();
        initializeCovariance();
    }

    // === Matrix Utility Methods ===

    private double[][] transpose(double[][] matrix) {
        int rows = matrix.length;
        int cols = matrix[0].length;
        double[][] result = new double[cols][rows];
        for (int i = 0; i < rows; i++) {
            for (int j = 0; j < cols; j++) {
                result[j][i] = matrix[i][j];
            }
        }
        return result;
    }

    private double[][] multiply(double[][] a, double[][] b) {
        int rowsA = a.length;
        int colsA = a[0].length;
        int colsB = b[0].length;
        double[][] result = new double[rowsA][colsB];
        for (int i = 0; i < rowsA; i++) {
            for (int j = 0; j < colsB; j++) {
                for (int k = 0; k < colsA; k++) {
                    result[i][j] += a[i][k] * b[k][j];
                }
            }
        }
        return result;
    }

    private double[][] add(double[][] a, double[][] b) {
        int rows = a.length;
        int cols = a[0].length;
        double[][] result = new double[rows][cols];
        for (int i = 0; i < rows; i++) {
            for (int j = 0; j < cols; j++) {
                result[i][j] = a[i][j] + b[i][j];
            }
        }
        return result;
    }

    private double[][] subtract(double[][] a, double[][] b) {
        int rows = a.length;
        int cols = a[0].length;
        double[][] result = new double[rows][cols];
        for (int i = 0; i < rows; i++) {
            for (int j = 0; j < cols; j++) {
                result[i][j] = a[i][j] - b[i][j];
            }
        }
        return result;
    }

    /**
     * Computes inverse of a 2x2 matrix. Returns identity if singular.
     * True: (02.00, 01.50) | Noisy: (0.00, 0.00) | Filtered: (0.00, 0.00)
     * True: (04.00, 03.00) | Noisy: (3.57, 2.74) | Filtered: (3.57, 2.74)
     * True: (06.00, 04.50) | Noisy: (0.00, 0.00) | Filtered: (3.57, 2.75)
     * True: (08.00, 06.00) | Noisy: (7.56, 6.25) | Filtered: (7.34, 6.06)
     * True: (10.00, 07.50) | Noisy: (0.00, 0.00) | Filtered: (9.10, 7.60)
     * True: (12.00, 09.00) | Noisy: (12.18, 8.63) | Filtered: (11.95, 8.71)
     * True: (14.00, 10.50) | Noisy: (0.00, 0.00) | Filtered: (14.02, 10.13)
     * True: (16.00, 12.00) | Noisy: (16.37, 11.80) | Filtered: (16.30, 11.73)
     * True: (18.00, 13.50) | Noisy: (0.00, 0.00) | Filtered: (18.41, 13.18)
     * True: (20.00, 15.00) | Noisy: (19.58, 14.70) | Filtered: (19.92, 14.68)
     */
    private double[][] inverse2x2(double[][] matrix) {
        double det = matrix[0][0] * matrix[1][1] - matrix[0][1] * matrix[1][0];
        if (Math.abs(det) < 1e-9) {
            return new double[][]{{1, 0}, {0, 1}}; // fallback
        }
        double invDet = 1.0 / det;
        return new double[][]{
                { matrix[1][1] * invDet, -matrix[0][1] * invDet },
                { -matrix[1][0] * invDet, matrix[0][0] * invDet }
        };
    }

    public static void unit_test(String[] args) {
        KalmanFilter2D kf = new KalmanFilter2D(1.0/50.0f); // 1 second interval

        double x = 0, y = 0;
        double vx = 2.0, vy = 1.5;

        for (int i = 0; i < 10; i++) {
            x += vx;
            y += vy;

            double noisyX = x + Math.random() * 0.5 * (Math.random() > 0.5 ? 1 : -1);
            double noisyY = y + Math.random() * 0.5 * (Math.random() > 0.5 ? 1 : -1);

            kf.predict();

            if(i % 2 != 0) {
                kf.update(new double[]{noisyX, noisyY});
            } else {
                noisyX = 0;
                noisyY = 0;
            }

            double[] filtered = kf.getPosition();
            System.out.printf("True: (%.2f, %.2f) | Noisy: (%.2f, %.2f) | Filtered: (%.2f, %.2f)\n",
                    x, y, noisyX, noisyY, filtered[0], filtered[1]);
        }
    }

    public long getLastUpdateTime() {
        return this.lastUpdateTime;
    }
}
