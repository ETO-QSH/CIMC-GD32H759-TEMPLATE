import torch.nn as nn


class TicTacToeCNN(nn.Module):
    def __init__(self):
        super().__init__()
        self.conv = nn.Sequential(
            nn.Conv2d(2, 16, kernel_size=3, padding=1),
            nn.ReLU(),
            nn.Flatten(),
            nn.Linear(16 * 3 * 3, 9)
        )

    def forward(self, x):
        return self.conv(x)
