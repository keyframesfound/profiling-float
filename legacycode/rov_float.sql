-- phpMyAdmin SQL Dump
-- version 5.2.1
-- https://www.phpmyadmin.net/
--
-- Host: 127.0.0.1
-- Generation Time: Apr 18, 2024 at 07:12 AM
-- Server version: 10.4.8-MariaDB
-- PHP Version: 8.1.17

SET SQL_MODE = "NO_AUTO_VALUE_ON_ZERO";
START TRANSACTION;
SET time_zone = "+00:00";


/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8mb4 */;

--
-- Database: `phd_demo`
--

-- --------------------------------------------------------

--
-- Table structure for table `shop_promos`
--

CREATE TABLE IF NOT EXISTS `rov_float` (
  `id` int(10) UNSIGNED NOT NULL AUTO_INCREMENT,
  `ssc_id` varchar(191) COLLATE utf8mb4_general_ci	 NOT NULL,
  `timezone` varchar(191) COLLATE utf8mb4_general_ci	 NOT NULL,
  `p` varchar(191) COLLATE utf8mb4_general_ci	 NOT NULL,
  `p_unit` varchar(191) COLLATE utf8mb4_general_ci	 NOT NULL,
  `d` varchar(191) COLLATE utf8mb4_general_ci	 NOT NULL,
  `d_unit` varchar(191) COLLATE utf8mb4_general_ci	 NOT NULL,
  `t` varchar(191) COLLATE utf8mb4_general_ci	 NOT NULL DEFAULT '--',
  `t_unit` varchar(191) COLLATE utf8mb4_general_ci	 NOT NULL DEFAULT '--',
  `a` varchar(191) COLLATE utf8mb4_general_ci	 NOT NULL DEFAULT '--',
  `a_unit` tinyint(1) NOT NULL DEFAULT 0,
  PRIMARY KEY (`id`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_general_ci	;
COMMIT;

/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
