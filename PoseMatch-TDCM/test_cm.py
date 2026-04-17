import warnings
warnings.filterwarnings("ignore")

import cv2, torch, math, random
from model import Model
from PIL import Image
import numpy as np
import matplotlib.pyplot as plt
import torchvision.transforms as transforms

from utils import draw_rotated_bbox, get_center
from utils.refine import *
from utils import getIOU

import time
import os
import glob


device = torch.device('cpu')

# dict/model.pth
model = Model('dict/dl1/model_epoch_50.pth').to(device).eval()

transform_img = transforms.Compose([
            transforms.ToTensor(),
            transforms.Normalize(mean=[0.485, 0.456, 0.406],  std=[0.229, 0.224, 0.225]),
        ])
        
def denormalize(tensor, mean=[0.485, 0.456, 0.406], std = [0.229, 0.224, 0.225]):
    mean = torch.tensor(mean).view(3, 1, 1)
    std = torch.tensor(std).view(3, 1, 1)
    return tensor * std + mean


dateRoot = "..\\Data"
resRoot = ".\\res\\tdcm"
vecTask = [dateRoot + "\\tra", dateRoot + "\\rot", dateRoot + "\\lig_rect", dateRoot + "\\lig_qb", dateRoot + "\\lig_qbc"]
vecFolderPath = []
for taskPath in vecTask:
    with os.scandir(taskPath + "\\") as entries:
      folderPaths = [entry.name for entry in entries if entry.is_dir()]
      for folderPath in folderPaths:
        vecFolderPath.append(taskPath + "\\" + folderPath)
    


for folderPath in vecFolderPath:
    # 模板绘制
    template_image_path = folderPath + "\\CompTpl.png"
    template = cv2.imread(template_image_path)
    template = cv2.resize(template, (36, 36))
    template = transform_img(Image.fromarray(template)).to(device).unsqueeze(0)
    
    folderItems = folderPath.split("\\")
    resPath = resRoot + "\\" + folderItems[-2]
    if not os.path.exists(resPath):
       os.makedirs(resPath)
    resFilePath = resPath +"\\" + folderItems[-1] + ".txt"
    file = open(resFilePath, 'w', encoding='utf-8')

    imgPaths = glob.glob(folderPath + "\\*.bmp")
    for i, imgPath in enumerate(imgPaths):
        query_image_path = imgPaths[i]
        image = cv2.imread(query_image_path)
        # roiWidth = int(224)
        # roiHeight = roiWidth
        # borWidth = int((image.shape[1] - roiWidth) / 2)
        # borHeight = int((image.shape[0] - roiHeight) / 2)
        # image = image[borHeight:borHeight+roiHeight, borWidth:borWidth + roiWidth, :].copy()
        img_ctr_x = (image.shape[1] - 1) * 0.5
        img_ctr_y = (image.shape[0] - 1) * 0.5
        image = transform_img(Image.fromarray(image)).to(device).unsqueeze(0)

        start_time =  time.perf_counter()
        pred_score, pred_sign, pred_cos, pred_scale_x, pred_scale_y = model(image, template)
        inference_time = time.perf_counter() - start_time
        
        # 计算 center
        pred = pred_score[0, 0, :, :]
        max_idx = torch.nonzero(pred == pred.max())[0]
        pre_Y, pre_X = max_idx.cpu().numpy()
        pred_y, pred_x = get_center((pre_Y, pre_X), torch.nonzero(pred > 0.5).cpu().numpy())
        
        # 计算 角度
        sign = 1 if pred_sign[0, 0, pre_Y, pre_X] > 0.5 else -1
        pred_r = torch.arccos(pred_cos[0, 0, pre_Y, pre_X].clamp(-1, 1)) * 180 / math.pi
        pred_r = sign * pred_r.item()
        
        # 计算缩放
        pred_sx = pred_scale_x[0, 0, pre_Y, pre_X].item()
        pred_sy = pred_scale_y[0, 0, pre_Y, pre_X].item()
        
        search_img = (denormalize(image.squeeze(0))).permute(1, 2, 0).cpu().numpy()
        template_img = (denormalize(template.squeeze(0))).permute(1, 2, 0).cpu().numpy()

        search_img = cv2.cvtColor(search_img, cv2.COLOR_BGR2GRAY)
        template_img = cv2.cvtColor(template_img, cv2.COLOR_BGR2GRAY)
        
        # 细化
        post_start = time.time()
        refined_angle, refine_score = refine_angle_bisection(
            search_img, template_img, pred_x, pred_y, pred_sx, pred_sy, pred_r,
        )
        pred_sx, _ = refine_scale_x(
            search_img, template_img, pred_x, pred_y, pred_sx, pred_sy, refined_angle,
            search_range=0.15, step_size=0.015
        )
        pred_sy, _ = refine_scale_y(
            search_img, template_img, pred_x, pred_y, pred_sx, pred_sy, refined_angle,
            search_range=0.15, step_size=0.015
        )
        # pred_r, _ = refine_angle_bisection(
        #     search_img, template_img, pred_x, pred_y, pred_sx, pred_sy, pred_r,
        #     initial_range=20,
        # )
        # pred_sx, _ = refine_scale_x(
        #     search_img, template_img, pred_x, pred_y, pred_sx, pred_sy, pred_r
        # )
        # pred_sy, _ = refine_scale_y(
        #     search_img, template_img, pred_x, pred_y, pred_sx, pred_sy, pred_r
        # )
        # pred_r, _ = refine_angle(
        #     search_img, template_img, pred_x, pred_y, pred_sx, pred_sy, pred_r,
        #     initial_range=5
        # )
        post_time = time.time() - post_start
        
        items1 = imgPaths[i].split("\\")
        fileName = items1[-1]
        
        
        wstr = fileName + " " + str(100.0) + " " + str((inference_time+post_time) * 1000) + " " + str(pred_x - img_ctr_x) + " " + str(pred_y - img_ctr_y) + " " + str(pred_r) + "\n"
        file.write(wstr)
        print(wstr)
    file.close()
